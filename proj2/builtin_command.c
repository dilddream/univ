#include "builtin_command.h"
#include "jobs.h"



/* If command is built-in, return 1. 
    In other case, return 0. */
int builtin_command(char* argv[]) {
    
    // cd command
    if (!strcmp(argv[0], "cd")) {
        if (argv[1] == NULL) 
            return (chdir(getenv("HOME")) + 1);
        else if (chdir(argv[1]) == 0)
            return 1;

        printf("cd: %s: No such file or directory\n", argv[1]);
        return 1;
    }

    // exit & quit command
    if (!strcmp(argv[0], "exit") || !strcmp(argv[0], "quit")) {
        exit(0);
    }

    // jobs command
    if (!strcmp(argv[0], "jobs")) {
        list_jobs();
        return 1;
    }

    return 0;
}