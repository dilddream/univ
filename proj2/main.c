#include "additional.h"
#include <termios.h>

pid_t shell_pgid;

int main() 
{
    // initialize shell process group and terminal control
    // Put shell in its own process group and grab control of terminal
    setpgid(0, 0);
    shell_pgid = getpid();
    Signal(SIGTTIN, SIG_IGN);
    Signal(SIGTTOU, SIG_IGN);
    tcsetpgrp(STDIN_FILENO, shell_pgid);

    // job control setup
    init_job_list();
    Signal(SIGCHLD, sigchld_handler);
    Signal(SIGINT,  sigint_handler);
    Signal(SIGTSTP, sigtstp_handler);
    atexit(clear_job);

    char cmdline[MAXLINE]; /* Command line */

    while (1) {
        cleanup_jobs();
        printf("CSE4100-SP-P2> ");                   
        if (fgets(cmdline, MAXLINE, stdin) == NULL) exit(0); 

        command(cmdline);
    }
}
