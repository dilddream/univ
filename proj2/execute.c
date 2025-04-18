#include "additional.h"

/* execute built-in-command.
    If success return 0
    otherwise return -1 */
int builtin_command(char **argv) {

    // cd command
    if (!strcmp(argv[0], "cd")) {
        if (argv[1] == NULL) { // cd without argument
            return chdir(getenv("HOME"));
        }
        else if (chdir(argv[1]) == 0) { // cd [directory]
            return 0;
        }

        printf("%s: No such file or directory\n", argv[1]);
        return 0;
    }

    // exit & quit command
    if (!strcmp(argv[0], "exit") || !strcmp(argv[0], "quit")) {
        exit(0);
    }

    // jobs command
    if (!strcmp(argv[0], "jobs")) {
        cleanup_jobs();
        list_job(job_list);
        return 0;
    }

    // bg, fg, kill builtins
    if (!strcmp(argv[0], "bg")) {
        do_bg(argv);
        return 0;
    }
    if (!strcmp(argv[0], "fg")) {
        do_fg(argv);
        return 0;
    }
    if (!strcmp(argv[0], "kill")) {
        do_kill(argv);
        return 0;
    }

    return -1;
}


/* execute command */
void command(char *cmdline) 
{
    char *argv[MAXARGS]; /* Argument list execve() */
    char buf[MAXLINE];   /* Holds modified command line */
    int bg;              /* Should the job run in bg or fg? */
    pid_t pid;           /* Process id */
    

    // parsing command line
    strcpy(buf, cmdline);
    bg = parseline(buf, argv); 

    // ignore empty line
    if (argv[0] == NULL) return;   



    // check pipeline command
    if (strchr(cmdline, '|') != NULL) {
        pipeline(cmdline, bg);
        return;
    }



    // check built-in command
    // (cd, exit, quit, jobs)
    if (!builtin_command(argv)) return;

    
    // check other command
    // error case
    if ((pid = fork()) < 0) { 
        fprintf(stderr, "fork error\n");
        return;
    }
    
    // child process
    if (pid == 0) { 
        setpgid(0, 0); // new process group
        // allow child to receive signals
        Signal(SIGINT, SIG_DFL);
        Signal(SIGTSTP, SIG_DFL);

        if (execvp(argv[0], argv) < 0) {
            fprintf(stderr, "%s: command not found\n", argv[0]);
            exit(1);
        }
    }

    // parent process
    int status;
    setpgid(pid, pid);

    if (!bg) {
        // foreground: take terminal control
        tcsetpgrp(STDIN_FILENO, pid);
    }

    pid_t pids[1] = {pid};


    // background
    if (bg) {
        add_job(pid, pids, 1, BG, cmdline);
        job_t *job = get_job_pgid(pid);
        printf("[%d] %d\n", job->job_id, job->pgid);
    }
    // foreground
    else {
        // wait for job to terminate or stop
        int status;
        waitpid(pid, &status, WUNTRACED);
        // restore control of terminal
        tcsetpgrp(STDIN_FILENO, shell_pgid);
        if (WIFSTOPPED(status)) {
            // add to job list as stopped
            pid_t p = pid;
            add_job(p, &p, 1, ST, cmdline);
            job_t *job = get_job_pgid(p);
            printf("[%d] Stopped   %s", job->job_id, job->cmdline);
        }
    }
}


