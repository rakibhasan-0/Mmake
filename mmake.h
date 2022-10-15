#ifndef MMAKE_H
#define MMAKE_H

#include <errno.h>
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


/**
 * @brief					It reads makefile rules from the specified file. And it checks if the user 
 * 						has inserted some flags which will determine if the cmds will be shown in stdin,
 * 						target will be forced build even the prerequisites file has not been updated yet.
 * 						Furthermore, it checks if the user has given a file and given targets in the 
 *						terminal.
 * 
 * @param filename				The filename, which indicates the users given filename name.
 * @param fp 					The file pointer.
 * @param flag_status 				It will contain the information if the user has specified some flags or not.
 * @param argc 	  				the command line counter.
 * @param argv 					the command line array.	
 * @return 	 				It will return the makefile, from the makefile we will create targets with the of
 *						its rule and prerequisites. If it cannot return the makefile then it will 
 *						return NULL.
 */
makefile* read_rules_from_file(char* filename, FILE** fp, flags* flag_status, int argc, char** argv); 



/**
 * @brief					It creates targets by checking the rule of that target, since each target
 * 						has some prerequisites thus that that function will do the depth-first search 
 *	 		 		 	check if those prerequisites has some rules or not. Furthermore, it will check
 *						some criteria to check whether if that function will build given target or not.
 * 
 * @param rules_recipes 			The make file which contains rules to build targets.
 * @param target 				The target name.
 * @param flag_status 				the flags info.
 */
void creation_of_targets(makefile* rules_recipes, const char* target, flags* flag_status); 

# endif
