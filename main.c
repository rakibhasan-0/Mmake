#include "mmake.h"
#include <stdio.h>

int main(int argc, char **argv){

	FILE* fp; 

	char* filename = NULL; 
	flags* flag_status = calloc(sizeof(flags),1); 
	
	makefile* make = read_rules_from_file(filename,&fp,flag_status, argc, argv);
	bool target_given = false;

	for(int index = optind; index < argc; index++){
		target_given = true;
		creation_of_targets(make,argv[index],flag_status); 
		flag_status->first_time = false;
	}

	if(!target_given){
		creation_of_targets(make,makefile_default_target(make),flag_status); 
	}

	fclose(fp);
	free(flag_status);
	makefile_del(make); 
	exit(EXIT_SUCCESS); 
	
}