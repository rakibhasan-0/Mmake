#include "parser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <sys/stat.h>

typedef struct flags {
	bool s_flag; 
	bool B_flag; 
}flags; 

void creating_targets (makefile* rules_recipes, const char* target, flags* flag_status); 
bool check_files_existence(const char* file_name); 
void print_cmd(rule* current_target, flags* flag_status);
bool check_modifications(const char* target, const char* prereq); 

int main(int argc, char **argv){

	int opt; 
	FILE* fp; 
	const char* target;
	char* filename = NULL; 
	flags* flag_status = calloc(sizeof(flags),1); 

//TODOwhat would happen if we don't given s in command args.

	while ((opt = getopt(argc, argv, "Bf:s")) != -1){
		switch (opt){
			case 'f':
				filename = optarg; 
				//fp = fopen(filename, "r");
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
	
	fp = fopen(filename ? filename : "mmakefile.txt", "r");

	bool target_given = false;
	makefile* make = parse_makefile(fp); 

	for(int index = optind; index < argc; index++){
		target = argv[index];
		target_given = true;
		creating_targets (make,argv[index],flag_status ); 
	}

	if(!target_given){
		target = makefile_default_target(make); 
		printf("%s\n", target);	
		creating_targets (make,target,flag_status); 
	}
	
}

void creating_targets (makefile* rules_recipes, const char* target, flags* flag_status){ 

	if(makefile_rule(rules_recipes, target) == NULL){
		return;
	}

	else{		

		int status;
		rule* make_rule = makefile_rule(rules_recipes, target); 
		const char** prerequisites = rule_prereq(make_rule); 
		
		struct stat target_status; 
		stat(target, &target_status);
		bool file_existence = check_files_existence(target);  

		int index = 0; 
		bool update_requires = false; 
		while (prerequisites[index] != NULL){
			creating_targets (rules_recipes, prerequisites[index], flag_status); 
			//printf("target %s, file_existence %d\n", target, file_existence); 
			struct stat prereq_status; 
			stat(prerequisites[index], &prereq_status);
			update_requires = check_modifications(target, prerequisites[index]);	
			index++; 
		}

		if( update_requires == true || !file_existence || flag_status->B_flag == true){
			
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
		else{
			printf("everything is upToDate\n");
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
