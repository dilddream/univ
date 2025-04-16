#include "pipeline.h"

void pipeline(char* cmdline) {

    static int first = 1;
    static int saved_stdin;

    if (first) {
        saved_stdin = dup(STDIN_FILENO);
        first = 0;
    }






    char *pipe_delim;
    char command[MAXLINE];

    strcpy(command, cmdline);
    pipe_delim = strchr(command, '|');



    /* --- Single Command --- */
    if (pipe_delim == NULL) {
        eval(cmdline);

        dup2(saved_stdin, STDIN_FILENO);
        close(saved_stdin);
        first = 1;
        return;
    }


    /* --- Multiple Commands --- */
    command[pipe_delim - command] = '\0';
    char *next_command = pipe_delim + 1;

    int fd[2];
    
    if (pipe(fd) < 0) {
        fprintf(stderr, "Pipe error\n");
        return;
    }


    pid_t pid = fork();

    /* Child process */
    if (pid == 0) {
        close(fd[READ_END]);
        dup2(fd[WRITE_END], STDOUT_FILENO);
        close(fd[WRITE_END]);

        eval(command);
        exit(0);
    }

    /* Parent process */
    close(fd[WRITE_END]);
    dup2(fd[READ_END], STDIN_FILENO);
    close(fd[READ_END]);

    waitpid(pid, NULL, 0);




    pipeline(next_command);



    return;
}