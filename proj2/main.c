#include "csapp.h"
#include "shellex.h"
#include "jobs.h"
#include "pipeline.h"
#include "builtin_command.h"

int main() 
{
    char cmdline[MAXLINE]; /* Command line */

    while (1) {
        /* Read */
        printf("CSE4100-SP-P2> ");                   
        if (fgets(cmdline, MAXLINE, stdin) == NULL) exit(0); 

        /* Evaluate */
        pipeline(cmdline);
    }

    atexit(clear_jobs);
}
