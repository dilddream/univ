#include "additional.h"

int main() 
{
    char cmdline[MAXLINE]; /* Command line */

    while (1) {
        printf("CSE4100-SP-P2> ");                   
        if (fgets(cmdline, MAXLINE, stdin) == NULL) exit(0); 

        command(cmdline);
    }

    atexit(clear_job);
}
