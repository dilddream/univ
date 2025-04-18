#include "additional.h"


/* Parse the command line and build the argv array */
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


/* Parse the pipeline and build the pipeline structure.
    If success return 0
    otherwise return -1 */
int parse_pipeline(char* cmdline, pipeline_t *pip) {
    char *commands[MAX_COMMANDS];
    char *cmd_curr = cmdline;
    char *pipe_curr;
    int n_commands = 0;
    

    // split command line by '|'
    while ((pipe_curr = strchr(cmd_curr, '|')) != NULL) {
        *pipe_curr = '\0';
        commands[n_commands++] = cmd_curr;
        cmd_curr = pipe_curr + 1;

        // skip leading spaces
        while (*cmd_curr && (*cmd_curr == ' ')) 
            cmd_curr++;
    }

    commands[n_commands++] = cmd_curr;

    // too many commands
    if (n_commands > MAX_COMMANDS) {
        fprintf(stderr, "Error: Too many commands in pipeline\n");
        return -1;
    }

    // pipeline structure init
    pip->n_commands = n_commands;
    pip->pgid = 0;
    pip->bg = 0;

    // parse each command
    for (int i=0; i<n_commands; i++) {
        command_t *cmd = &pip->commands[i];
        int is_bg = parseline(commands[i], cmd->argv);

        if (i == n_commands - 1) {
            pip->bg = is_bg;
        }

        int argc = 0;
        while (cmd->argv[argc] != NULL) 
            argc++;
        cmd->argc = argc;
    }

    return 0;
}


/* handle pipeline command */
void pipeline(char* cmdline, int bg) {

    pipeline_t pipe;
    pid_t *pids;

    // parse pipeline
    parse_pipeline(cmdline, &pipe);

    // pids allocation
    pids = (pid_t*)malloc(sizeof(pid_t) * pipe.n_commands);
    if (pids == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return;
    }
    
    // execute pipeline
    execute_pipeline(&pipe, 0, pids);

    // add job to job list
    add_job(pids[0], pids, pipe.n_commands, pipe.bg ? BG : FG, cmdline);

    if (!pipe.bg) {
        waitpid(pids[0], NULL, 0);
    }
    else {
        printf("[%d] %d\n", get_job_jid(pids[0])->job_id, pids[0]);
    }
}


/* execute pipeline commands recursively
    If success return 0
    otherwise return -1 */
int execute_pipeline(pipeline_t *pip, int cmd_idx, pid_t *pids) {
    int pipefd[2];
    pid_t pid;
    
    
    if (cmd_idx == pip->n_commands - 1) {
        pid = fork();
        if (pid == 0) {
            setpgid(0, pip->pgid);
            setbuf(stdout, NULL);  // disable buffering
            execvp(pip->commands[cmd_idx].argv[0], 
                  pip->commands[cmd_idx].argv);
            exit(1);
        }
        pids[cmd_idx] = pid;
        waitpid(pid, NULL, 0);

        return 0;
    }
    
    // create pipe
    if (pipe(pipefd) < 0) {
        fprintf(stderr, "Pipe creation error\n");
        return -1;
    }
    
    // execute command
    pid = fork();
    if (pid == 0) {  // child process
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        
        setpgid(0, pip->pgid);
        execvp(pip->commands[cmd_idx].argv[0], 
               pip->commands[cmd_idx].argv);
        exit(1);
    }
    
    // parent process
    if (cmd_idx == 0) {
        pip->pgid = pid;
    }
    setpgid(pid, pip->pgid);
    pids[cmd_idx] = pid;

    int stdin_copy = dup(STDIN_FILENO);
    // connect pipe
    close(pipefd[1]);
    dup2(pipefd[0], STDIN_FILENO);
    close(pipefd[0]);
    
    int result = execute_pipeline(pip, cmd_idx + 1, pids);
    // restore stdin
    dup2(stdin_copy, STDIN_FILENO);
    close(stdin_copy);

    return result;
}