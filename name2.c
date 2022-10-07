#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "parser.h"

void stat_s(const char *file, struct stat *sb)
{
    if(access(file, F_OK) == 0) {
        if (stat(file, sb) == -1) {
            perror("stat unsuccessful");
            fprintf(stderr, "for [%s]\n", file);
            exit(EXIT_FAILURE);
        }
    }
}

pid_t fork_s(){
    pid_t p = fork();
    if(p == -1)
    {
        perror("fork unsuccessful");
        exit(EXIT_FAILURE);
    }
    return p;
}

FILE *fopen_s(char *filename, char *mode)
{
    FILE *fp = fopen(filename, mode);
    if(!fp) {
        fprintf(stderr, "%s: ", filename);
        perror("");
        exit(EXIT_FAILURE);
    }
    return fp;
}

void print_cmd(char **cmd)
{
    for(int i = 0; cmd[i]; i++)
    {
        printf("%s", cmd[i]);
        if(cmd[i + 1])
        {
            printf(" ");
        }
    }
    printf("\n");
}

void build_target(makefile *mf, const char *target, int force, int nocmd)
{
    rule *aRule = makefile_rule(mf, target);
    if(!aRule) {
        if(access(target, F_OK) != 0)
        {
            fprintf(stderr, "mmake: No rule to make target '%s'\n", target);
            exit(EXIT_FAILURE);
        }
        return;
    }

    const char **pReq = rule_prereq(aRule);
    int target_outdated = 0;

    // target
    struct stat sb_target = {};
    stat_s(target, &sb_target);

    int itr = 0;
    while(pReq[itr])
    {
        // try build any prerequisites
        build_target(mf, pReq[itr], force, nocmd);

        struct stat sb = {};
        stat_s(pReq[itr], &sb);

        if (sb_target.st_mtime < sb.st_mtime)
        {
            target_outdated = 1;
        }

        // sb ...
        itr++;
    }

    pid_t p = -1;

    if(force || target_outdated || (access(target, F_OK) == -1))
    {
        p = fork_s();
    }

    if(p == 0)
    {
        char **cmd = rule_cmd(aRule);

        if(!nocmd) {
            print_cmd(cmd);
            fflush(stdout);
        }

        if(execvp(cmd[0], cmd)){
            fprintf(stderr, "%s: ", cmd[0]);
            perror("");
            exit(EXIT_FAILURE);
        }
    }

    if(p > 0)
    {
        int status;
        waitpid(p, &status, WUNTRACED);

        if (WEXITSTATUS(status)) {
            //fprintf(stderr, "child process [%d] failed\n", p);
            exit(EXIT_FAILURE);
        }
    }

}


int main(int argc, char *argv[]) {

    FILE *input = NULL;
    char *filename = NULL;

    int force_build = 0;
    int no_cmd_stdout = 0;

    int opt;
    while((opt = getopt(argc, argv, "f:Bs")) != -1)
    {
        switch (opt) {
            case 'f':
                filename = malloc(strlen(optarg) + 1);
                strcpy(filename, optarg);
                input = fopen_s(filename, "r");
                break;
            case 'B':
                force_build = 1;
                break;
            case 's':
                no_cmd_stdout = 1;
                break;
            case '?':
                fprintf(stderr, "Unknown option issued\n");
                exit(EXIT_FAILURE);
        }
    }

    if(input == NULL)
    {
        filename = malloc(strlen("mmakefile") + 1);
        strcpy(filename, "mmakefile");
        input = fopen_s(filename, "r");
    }

    makefile *make = parse_makefile(input);
    if(make == NULL)
    {
        fprintf(stderr, "%s: Could not parse makefile\n", filename);
        exit(EXIT_FAILURE);
    }

    int partial_build = 0;
    while(optind < argc)
    {
        partial_build = 1;
        build_target(make, argv[optind], force_build, no_cmd_stdout);
        optind++;
    }

    if(!partial_build) {
        build_target(make, makefile_default_target(make), force_build, no_cmd_stdout);
    }

    makefile_del(make);
    fclose(input);
    exit(EXIT_SUCCESS);
}


