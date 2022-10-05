#include "parser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <sys/stat.h>

typedef struct flags{
	bool s_flag; 
	bool B_flag; 
	bool first_time; 
}flags; 

void creation_of_targets(makefile* rules_recipes, const char* target, flags* flag_status); 
bool check_files_existence(const char* file_name); 
void print_cmd(rule* current_target, flags* flag_status);
bool check_modifications(const char* target, const char* prereq); 
void creates_new_process_executes_cmd(bool update_requires, bool file_existence, flags* flag_status, rule* make_rule); 
makefile* read_rules_from_file(char* filename, FILE** fp, flags* flag_status, int argc, char** argv); 

int main(int argc, char **argv){

	FILE* fp; 
	const char* target;

	char* filename = NULL; 
	flags* flag_status = calloc(sizeof(flags),1); 
	
	makefile* make = read_rules_from_file(filename,&fp,flag_status, argc, argv);
	bool target_given = false;

	for(int index = optind; index < argc; index++){
		target = argv[index];
		target_given = true;
		creation_of_targets(make,argv[index],flag_status ); 
		flag_status->first_time = false;
	}

	if(!target_given){
		target = makefile_default_target(make); 
		creation_of_targets(make,target,flag_status); 
	}

	fclose(fp);
	free(flag_status);
	makefile_del(make); 
	exit(EXIT_SUCCESS); 
	
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
	makefile* make = parse_makefile(*fp); 

	return make; 
}


void creation_of_targets(makefile* rules_recipes, const char* target, flags* flag_status){ 

	if(makefile_rule(rules_recipes, target) == NULL){ // base case
		if(!flag_status->first_time){
			fprintf(stderr, "there exists no rule for %s\n", target); 
			flag_status->first_time = true;
			exit(EXIT_FAILURE);
		}
		return;
	}

	else{		

		rule* make_rule = makefile_rule(rules_recipes, target);
		flag_status->first_time = true; 
		//printf("size of make rule %ld\n",sizeof(make_rule)); 
		const char** prerequisites = rule_prereq(make_rule); 
		bool file_existence = check_files_existence(target);  

		int index = 0; 
		bool update_requires = false; 

		while (prerequisites[index] != NULL){
			creation_of_targets(rules_recipes, prerequisites[index], flag_status); 
			update_requires = check_modifications(target, prerequisites[index]);	
			index++; 
		}

		creates_new_process_executes_cmd(update_requires,file_existence,flag_status,make_rule); 
		
	}

}

void creates_new_process_executes_cmd(bool update_requires, bool file_existence, flags* flag_status, rule* make_rule){

	int status;
	if( update_requires|| !file_existence || flag_status->B_flag){
			
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


bool check_modifications(const char* target, const char* prereq){

	struct stat st_target; 
	stat(target, &st_target); 

	struct stat st_prereq;
	stat(prereq, &st_prereq); 

	if(st_target.st_mtime < st_prereq.st_mtime){
		return true; 
	}
	return false; 
}

bool check_files_existence(const char* file_name){
	struct stat buffer; 
	return (stat(file_name, &buffer) == 0);
}

void print_cmd(rule* current_target, flags* flag_status){
	if(flag_status->s_flag == false){
		char** cmds = rule_cmd(current_target); 
		int i = 0; 
		while(cmds[i] != NULL){
			printf("%s ",cmds[i]); 
			i++; 
		}
		printf("\n");
	}
	return; 
}
