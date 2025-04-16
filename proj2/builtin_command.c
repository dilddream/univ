#include "builtin_command.h"

int builtin_command(char* argv[]) {
    
    // cd command
    if (!strcmp(argv[0], "cd")) {
        if (argv[1] == NULL) 
            chdir(getenv("HOME"));
        else if (chdir(argv[1]) != 0)
            printf("%s: No such file or directory\n", argv[1]);

        return 1;
    }
    // exit command
    if (!strcmp(argv[0], "exit")) {
        exit(0);
    }
    // quit command
    if (!strcmp(argv[0], "quit")) {
        exit(0);
    }
    // ignore singleton &
    if (!strcmp(argv[0], "&")) {
        return 1;
    }

    // other built-in command

    return 0;
}