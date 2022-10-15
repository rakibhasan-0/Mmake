#include <errno.h>
#include "mmake.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <sys/stat.h>


typedef struct flags flags;


/**
 * @brief			 			It checks if the file exits in the directory or not.
 * 
 * @param file_name			 		The name of the file.
 * @return 	 				 	It returns true if the file exists, otherwise it returns false.
 */
static bool check_files_existence(const char* file_name){
	if(access(file_name,F_OK) == 0){
		return true;
	}
	return false;
}




/**
 * @brief				 		It prints the commands which will be executed.
 *
 * @param current_target	  	 		the current target.
 * @param flag_status 	  				it checks if the -s flag is given or not.
 */
static void print_cmd(rule* current_target, flags* flag_status){

	if(flag_status->s_flag == false){

		char** cmds = rule_cmd(current_target);
		int i = 0;
		printf("%s",cmds[i++]);

		while(cmds[i] != NULL){
			printf(" %s",cmds[i]);
			i++;
		}
		printf("\n");
	}
	return;
}


/**
 * @brief				 	It checks if the specified files has been modified or not.
 *
 * @param target		    		the target file name.
 * @param prereq 	  			The prerequisites file name.
 * @return		 	 		It returns true if the prerequisites file has been modified, otherwise false.
 */
static bool check_modifications(const char* target, const char* prereq){

	struct stat st_target; 
	if(stat(target, &st_target)!=0){
		//fprintf(stderr, "file %s, %s\n", filepath,strerror(errno));
		return false; 
	}

	struct stat st_prereq;
	if(stat(prereq, &st_prereq)!=0){
		//fprintf(stderr, "%s, %s\n", filepath,strerror(errno));
		return false; 
	} 

//	printf("target: %ld, prereq: %ld\n", st_target.st_mtime, st_prereq.st_mtime);
	if(st_target.st_mtime < st_prereq.st_mtime){
		return true; 
	}
	return false; 
}





/**
 * @brief 					By creating new processes it executes target, to execute targets it 
 *						checks for certain criteria to determine if the target gets executed or not.
 * 
 * @param update_requires		  	that flags checks if target files prerequisites has been updated.
 * @param file_existence	  	  	that flags checks if target files exists or not. we will execute the target if the 
 * 						target fil does not exists.

 * @param flag_status		 	  	it contains the information if the user have inserted B-flag or not.
 * @param make_rule				It contains the information about the target's rule.
*/
static void creates_new_process_executes_cmd(bool update_requires, bool file_existence, flags* flag_status, rule* make_rule){

	int status;
	if( update_requires == true || !file_existence || flag_status->B_flag){

		print_cmd(make_rule,flag_status);

		int pid = fork();
		if(pid < 0){
			perror("something went wrong with fork\n");
			exit(EXIT_FAILURE);
		}
		if(pid == 0){
			char** get_cmds = rule_cmd(make_rule);
			if(execvp(get_cmds[0],get_cmds) < 0){
				perror("execvp failed\n");
				exit(EXIT_FAILURE);
			}
		}
		if(pid > 0){
			wait(&status);
			if(WEXITSTATUS(status) != 0){
				perror("WEXITSTATUS failed\n");
				exit(EXIT_FAILURE);
			}
		}
	}

}


makefile* read_rules_from_file(char* filename, FILE** fp, flags* flag_status, int argc, char** argv){

	int opt; 
	while ((opt = getopt(argc, argv, "Bf:s")) != -1){
		switch (opt){
			case 'f':
				filename = optarg; 
				break;
			case ':': 
				printf("it requires argument\n");
				exit(EXIT_FAILURE); 
				break; 
			case 'B':
				flag_status->B_flag = true; 
				break; 
			case 's': 
				flag_status->s_flag = true; 
				break; 
			case '?':
				printf("unknown option\n"); 
				exit(EXIT_FAILURE); 
				break;
		}
	}

	*fp = fopen(filename ? filename : "mmakefile", "r");
	if(*fp == NULL){
		fprintf(stderr,"mmakefile: No such file or directory\n"); 
		exit(EXIT_FAILURE);
	}
	makefile* make = parse_makefile(*fp); 
	if(make == NULL){
		fprintf(stderr,"mmakefile: Could not parse makefile\n");
		exit(EXIT_FAILURE);
	}

	return make; 
}




void creation_of_targets(makefile* rules_recipes, const char* target, flags* flag_status){ 

	if(makefile_rule(rules_recipes, target) == NULL){ // base case
		if(!flag_status->first_time){
			flag_status->first_time = true;
		}
		if(check_files_existence(target) == false){
			fprintf(stderr, " mmake: no rule to make target %s\n", target); 
			exit(EXIT_FAILURE);
		}
		
		return;
	}

	else{

		rule* make_rule = makefile_rule(rules_recipes, target);
		flag_status->first_time = true;

		const char** prerequisites = rule_prereq(make_rule);
		bool file_existence = check_files_existence(target);

		int index = 0;
		bool update_requires = false;

		while (prerequisites[index] != NULL){

			creation_of_targets(rules_recipes, prerequisites[index], flag_status);
			bool check_files = check_files_existence(prerequisites[index]);		
			check_files = check_files_existence(target);

			if(check_files == true && !update_requires){
				update_requires = check_modifications(target, prerequisites[index]);
			}

			index++;
		}

		creates_new_process_executes_cmd(update_requires,file_existence,flag_status,make_rule);

	}

}
