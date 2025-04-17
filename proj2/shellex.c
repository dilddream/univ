#include "shellex.h"

extern int builtin_command(char* argv[]);
extern int add_job(pid_t pgid, int state, char* cmdline);
extern int get_job_id(pid_t pgid);

/* $begin eval */
void eval(char *cmdline) 
{
    char *argv[MAXARGS]; /* Argument list execve() */
    char buf[MAXLINE];   /* Holds modified command line */
    int bg;              /* Should the job run in bg or fg? */
    pid_t pgid;           /* Process id */
    

    /* --- Parse command argv --- */
    strcpy(buf, cmdline);
    bg = parseline(buf, argv); 

    /* --- execute command --- */
    /* Ignore empty lines */
    if (argv[0] == NULL)  
	    return;   

    /* --- built-in command --- */
    if (!builtin_command(argv)) { // cd, exit, quit, &

        /* --- Non-built-in command --- */

        /* Child process execute and return 0.
            If invalid command, return 1.          */
        if ((pgid = fork()) == 0) {
            if (execvp(argv[0], argv) < 0) {
                fprintf(stderr, "%s: command not found\n", argv[0]);
                exit(1);
            }
        }


        /* Parent process */
        int status;

        if (bg) {
            if (waitpgid(pgid, &status, WNOHANG) >= 0) {
                usleep(100);
                if ((WIFEXITED(status) && WEXITSTATUS(status)) != 0) {
                    return;
                }
            }
            add_job(pgid, bg, cmdline);
            printf("[%d] %d\n", get_job_id(pgid), pgid);
            return;
        }
        else {
            if (waitpgid(pgid, &status, 0) < 0) {
                fprintf(stderr, "waitfg: waitpgid error\n");
            }
        }

    }
}






/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv) 
{
    char *delim;         /* Points to first space delimiter */
    int argc;            /* Number of args */
    int bg;              /* Background job? */

    buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
	    buf++;

    /* Build the argv list */
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' ')) /* Ignore spaces */
            buf++;
    }
    argv[argc] = NULL;
    

    if (argc == 0)  /* Ignore blank line */
	    return 1;

    /* Should the job run in the background? */
    char *last_arg = argv[argc-1];
    int last_len = strlen(last_arg);

    if (last_len > 0 && last_arg[last_len-1] == '&') {
        bg = 1;

        if (last_len == 1)
            argv[--argc] = NULL;
        else
            last_arg[last_len-1] = '\0';
    }
    else
        bg = 0;

    

    return bg;
}
/* $end parseline */


