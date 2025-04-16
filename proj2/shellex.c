#include "shellex.h"

/* $begin shellmain */
int main() 
{
    char cmdline[MAXLINE]; /* Command line */

    while (1) {
        /* Read */
        printf("CSE4100-SP-P2> ");                   
        fgets(cmdline, MAXLINE, stdin); 
        if (feof(stdin))
            exit(0);

        /* Evaluate */
        eval(cmdline);
    } 
}
/* $end shellmain */
  


/* $begin eval */
void eval(char *cmdline) 
{
    char *argv[MAXARGS]; /* Argument list execve() */
    char buf[MAXLINE];   /* Holds modified command line */
    int bg;              /* Should the job run in bg or fg? */
    pid_t pid;           /* Process id */
    

    /* --- Parse command argv --- */
    strcpy(buf, cmdline);
    bg = parseline(buf, argv); 


    /* --- execute command --- */

    /* Ignore empty lines */
    if (argv[0] == NULL)  
	    return;   

    /* built-in command */
    if (!builtin_command(argv)) { // cd, exit, quit, &

    
    /* */
        if (execve(argv[0], argv, environ) < 0) {	//ex) /bin/ls ls -al &
            printf("%s: Command not found.\n", argv[0]);
            exit(0);
        }









        /* Parent waits for foreground job to terminate */
        if (!bg){ 
            int status;
        }
        else//when there is backgrount process!
            printf("%d %s", pid, cmdline);
    }
    return;
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
    if ((bg = (*argv[argc-1] == '&')) != 0)
	argv[--argc] = NULL;

    return bg;
}
/* $end parseline */


