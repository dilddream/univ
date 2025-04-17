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
        list_job(job_list);
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

        if (execvp(argv[0], argv) < 0) {
            fprintf(stderr, "%s: command not found\n", argv[0]);
            exit(1);
        }
    }

    // parent process`
    int status;
    setpgid(pid, pid);

    pid_t pids[1] = {pid};


    // background
    if (bg) {
        add_job(pid, pids, 1, BG, cmdline);
        printf("[%d] %d\n", 
            get_job_jid(pid)->job_id, pid);
    }
    // foreground
    else {
        add_job(pid, pids, 1, FG, cmdline);
        waitpid(pid, &status, 0);
    }
}


