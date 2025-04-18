#include "additional.h"


/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv) 
{
    char *delim;         /* Points to first space delimiter */
    int argc;            /* Number of args */
    int bg;              /* Background job? */

    int len = strlen(buf);
    if (len > 0 && buf[len-1] == '\n')
        buf[len-1] = ' ';
    else
        ; // no newline, leave buffer unchanged

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
    // Add last token if remaining
    if (*buf != '\0')
        argv[argc++] = buf;
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
    char *cmd_copy = strdup(cmdline);
    // Remove trailing newline
    cmd_copy[strcspn(cmd_copy, "\n")] = '\0';
    // Split into individual commands
    char *commands[MAX_COMMANDS];
    int n = 0;
    char *token = strtok(cmd_copy, "|");
    while (token && n < MAX_COMMANDS) {
        // Trim leading spaces
        while (*token == ' ') token++;
        commands[n++] = token;
        token = strtok(NULL, "|");
    }
    // Create pipes
    int pipefd[MAX_COMMANDS-1][2];
    for (int i = 0; i < n-1; i++) {
        if (pipe(pipefd[i]) < 0) unix_error("pipe error");
    }
    pid_t pids[MAX_COMMANDS];
    pid_t pgid = 0;
    // Launch processes
    for (int i = 0; i < n; i++) {
        char *argv[MAXARGS];
        parseline(commands[i], argv);
        pid_t pid = fork();
        if (pid < 0) unix_error("fork error");
        if (pid == 0) {
            // Child
            if (i == 0) pgid = getpid();
            setpgid(0, pgid);
            // reset signals to default in child
            Signal(SIGINT, SIG_DFL);
            Signal(SIGTSTP, SIG_DFL);
            // Redirect input
            if (i > 0) {
                dup2(pipefd[i-1][READ_END], STDIN_FILENO);
            }
            // Redirect output
            if (i < n-1) {
                dup2(pipefd[i][WRITE_END], STDOUT_FILENO);
            }
            // Close all pipes
            for (int j = 0; j < n-1; j++) {
                close(pipefd[j][READ_END]);
                close(pipefd[j][WRITE_END]);
            }
            execvp(argv[0], argv);
            fprintf(stderr, "%s: command not found\n", argv[0]);
            exit(1);
        }
        // Parent: set pgid and record pid
        if (i == 0) pgid = pid;
        setpgid(pid, pgid);
        pids[i] = pid;
    }
    // Parent: close all pipes
    for (int i = 0; i < n-1; i++) {
        close(pipefd[i][READ_END]);
        close(pipefd[i][WRITE_END]);
    }
    if (bg) {
        // background: add job and notify
        add_job(pgid, pids, n, BG, cmdline);
        job_t *job = get_job_pgid(pgid);
        printf("[%d] %d\n", job->job_id, job->pgid);
    } else {
        // foreground: give terminal to job
        tcsetpgrp(STDIN_FILENO, pgid);
        // wait for all processes, catch stop
        int stopped = 0;
        for (int i = 0; i < n; i++) {
            int status;
            waitpid(pids[i], &status, WUNTRACED);
            if (WIFSTOPPED(status)) stopped = 1;
        }
        // restore terminal to shell
        tcsetpgrp(STDIN_FILENO, shell_pgid);
        if (stopped) {
            // add as stopped job
            add_job(pgid, pids, n, ST, cmdline);
            job_t *job = get_job_pgid(pgid);
            printf("[%d] Stopped   %s", job->job_id, job->cmdline);
        }
    }
    free(cmd_copy);
}