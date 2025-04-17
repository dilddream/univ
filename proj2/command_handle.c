#include "additional.h"


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


void pipeline(char* cmdline, int bg) {

    return;
}