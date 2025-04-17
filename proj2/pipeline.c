#include "pipeline.h"
#include "shellex.h"
#include "jobs.h"

void init_pgid_array(pgid_array_t *arr) {
    arr->pgids = malloc(sizeof(pid_t) * INITIAL_SIZE);
    arr->size = INITIAL_SIZE;
    arr->count = 0;
}

void add_pgid(pgid_array_t *arr, pid_t pgid) {
    if (arr->count == arr->size) {
        arr->size *= 2;
        arr->pgids = realloc(arr->pgids, sizeof(pid_t) * arr->size);
    }
    arr->pgids[arr->count++] = pgid;
}

void free_pgid_array(pgid_array_t *arr) {
    free(arr->pgids);
    arr->pgids = NULL;
    arr->size = 0;
    arr->count = 0;
}

void pipeline(char* cmdline) {
    static pgid_array_t pgid_array;
    static pid_t pgid = 0;
    static int first = 1;
    static int saved_stdin;
    int bg;

    if (first) {
        saved_stdin = dup(STDIN_FILENO);
        init_pgid_array(&pgid_array);
        pgid = 0;
        first = 0;
    }

    char *pipe_delim;
    char command[MAXLINE];

    strcpy(command, cmdline);
    pipe_delim = strchr(command, '|');

    /* --- Single Command --- */
    if (pipe_delim == NULL) {
        char *argv[MAXARGS];
        bg = parseline(command, argv);
        
        if (argv[0] == NULL) {
            if (pgid_array.count > 0) {
                add_pipeline_job(pgid_array.pgids[0], pgid_array.pgids, 
                               pgid_array.count, BG, cmdline);
                free_pgid_array(&pgid_array);
                init_pgid_array(&pgid_array);
            }
            return;
        }

        pid_t pgid;
        if ((pgid = fork()) == 0) {
            if (pgid == 0) pgid = getpid();
            setpgid(0, pgid);
            
            if (execvp(argv[0], argv) < 0) {
                fprintf(stderr, "%s: Command not found\n", argv[0]);
                exit(1);
            }
        }

        /* Parent process */
        if (pgid == 0) pgid = pgid;
        setpgid(pgid, pgid);
        add_pgid(&pgid_array, pgid);

        if (pgid_array.count > 0) {
            if (!bg) {
                waitpgid(pgid, NULL, 0);
            }
            add_pipeline_job(pgid_array.pgids[0], pgid_array.pgids, 
                           pgid_array.count, bg ? BG : FG, cmdline);
            printf("[%d] (%d) %s", get_job_id(pgid_array.pgids[0]), 
                   pgid_array.pgids[0], cmdline);
        }

        free_pgid_array(&pgid_array);
        init_pgid_array(&pgid_array);
        pgid = 0;
        
        dup2(saved_stdin, STDIN_FILENO);
        close(saved_stdin);
        first = 1;
        return;
    }

    /* --- Multiple Commands --- */
    command[pipe_delim - command] = '\0';
    char *next_command = pipe_delim + 1;

    int fd[2];
    if (pipe(fd) < 0) {
        fprintf(stderr, "Pipe error\n");
        return;
    }

    /* Child process */
    pid_t pgid = fork();
    if (pgid == 0) {
        if (pgid == 0) pgid = getpid();
        setpgid(0, pgid);
        
        close(fd[READ_END]);
        dup2(fd[WRITE_END], STDOUT_FILENO);
        close(fd[WRITE_END]);

        char *argv[MAXARGS];
        parseline(command, argv);
        if (execvp(argv[0], argv) < 0) {
            fprintf(stderr, "%s: Command not found\n", argv[0]);
            exit(1);
        }
    }

    /* Parent process */
    if (pgid == 0) pgid = pgid;
    setpgid(pgid, pgid);
    add_pgid(&pgid_array, pgid);

    close(fd[WRITE_END]);
    dup2(fd[READ_END], STDIN_FILENO);
    close(fd[READ_END]);

    waitpgid(pgid, NULL, 0);

    pipeline(next_command);
    return;
}