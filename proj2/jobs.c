#include "additional.h"

job_t* job_list = NULL;


/* initialize job list */
void init_job_list(void) {
    job_list = NULL;
}

/* clear job list */
void clear_job(void) {
    job_t* curr = job_list;

    while (curr != NULL) {
        job_t* next = curr->next;
        if (curr->pids != NULL)
            free(curr->pids);
        free(curr);
        curr = next;
    }
    job_list = NULL;
}


/* add job to list.
    If success return 0
    otherwise return -1 */
int add_job(pid_t pgid, pid_t *pids, int n_processes, int state, char *cmdline) {
    
    // create new job node
    job_t* new_job = (job_t*)malloc(sizeof(job_t));
    if (new_job == NULL) { // fail
        fprintf(stderr, "Memory allocation error\n");
        return -1;
    }
    

    // set job's basic info
    new_job->pgid = pgid;
    new_job->pids = malloc(sizeof(pid_t) * n_processes);
    if (new_job->pids == NULL) { // fail
        free(new_job);
        return -1;
    }
    memcpy(new_job->pids, pids, sizeof(pid_t) * n_processes);
    new_job->n_processes = n_processes;


    // insert new job to the list
    new_job->next = job_list;
    job_list = new_job;


    // set job's info
    if (new_job->next == NULL) new_job->job_id = 1;
    else new_job->job_id = new_job->next->job_id + 1;
    new_job->state = state;
    strcpy(new_job->cmdline, cmdline);
    
    return 0;
}

/* delete job in list.
    If success return 0
    otherwise return -1 */
int delete_job(pid_t pgid) {
    job_t* curr = job_list;
    job_t* prev = NULL;
    
    while (curr != NULL) {
        if (curr->pgid == pgid) {
            if (prev == NULL) job_list = curr->next; // disconnect first job
            else prev->next = curr->next;            // disconnect job
            
            if (curr->pids != NULL) free(curr->pids); // free pids
            free(curr);                               // free job node
            return 0;
        }
        prev = curr;
        curr = curr->next;
    }
    return -1;
}



/* return job for specific pgid 
    If fail, return NULL */
job_t* get_job_pgid(pid_t pgid) {
    job_t* curr = job_list;
    
    while (curr != NULL) {
        if (curr->pgid == pgid)
            return curr;
        curr = curr->next;
    }
    return NULL;
}

/* return job for specific job_id
    If fail, return NULL */
job_t* get_job_jid(int job_id) {
    job_t* curr = job_list;
    
    while (curr != NULL) {
        if (curr->job_id == job_id)
            return curr;
        curr = curr->next;
    }
    return NULL;
}



/* update specific job's state.
    If success return 0
    otherwise return -1 */
int update_job_state(pid_t pgid, int state) {
    job_t* job = get_job_pgid(pgid);
    if (job == NULL) return -1;  // job not found

    if (state != BG && state != ST && state != FG) return -1;  // wrong state

    job->state = state;
    return 0;
}

/* check if job is completed
   If job is completed, return 1
   If job is still running, return 0
   otherwise return -1 */
int job_is_completed(pid_t pgid) {
    job_t* job = get_job_pgid(pgid);
    if (job == NULL) return -1;     // job not found


    // check all processes in the job
    for (int i = 0; i < job->n_processes; i++) {
        int status;
        pid_t pid = job->pids[i];

        pid_t result = waitpid(pid, &status, WNOHANG);
        
        if (result == -1) { //error case
            if (errno == ECHILD) continue;
            return -1;
        }
        else if (result == 0) { // still running
            return 0;
        }
        else if (result == pid) { // not completed
            if (!WIFEXITED(status) && !WIFSIGNALED(status)) return 0;
        }
        // other case
    }

    return 1;  // all processes are completed
}

/* check if job is stopped
   If job is stopped, return 1
   If job is running, return 0
   otherwise return -1 */
int job_is_stopped(pid_t pgid) {
    job_t* job = get_job_pgid(pgid);
    if (job == NULL) return -1;     // job not found

    // check all processes in the job
    for (int i = 0; i < job->n_processes; i++) {
        int status;
        pid_t pid = job->pids[i];

        pid_t result = waitpid(pid, &status, WNOHANG);
        
        if (result == -1) {         // error case
            if (errno == ECHILD) continue;
            return -1;
        }
        else if (result == 0) {     // still running
            return 0;
        }
        else if (result == pid) {   // process state changed
            if (!WIFSTOPPED(status)) return 0;  // not stopped
        }
    }

    return 1;  // all processes are stopped
}



/* list all jobs.
    Usage : list_job(job_list) */
void list_job(job_t* job) {
    if (job == NULL) return;

    list_job(job->next);

    // print job info
    printf("[%d]   ", job->job_id);
    switch (job->state) {
        case BG:
        case FG:
            printf("Running");
            break;
        case ST: 
            printf("Stopped");
            break;
        default:
            printf("Unknown");
            break;
    }
    printf("            %s", job->cmdline);
}

// get current foreground job
job_t* get_fg_job(void) {
    job_t* j = job_list;
    while (j) {
        if (j->state == FG)
            return j;
        j = j->next;
    }
    return NULL;
}

// SIGCHLD handler: reap children, update or delete jobs
void sigchld_handler(int sig) {
    int olderrno = errno;
    pid_t pid;
    int status;
    while ((pid = waitpid(-1, &status, WNOHANG|WUNTRACED|WCONTINUED)) > 0) {
        pid_t pgid = getpgid(pid);
        job_t* job = get_job_pgid(pgid);
        if (!job) continue;
        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            delete_job(pgid);
        } else if (WIFSTOPPED(status)) {
            update_job_state(pgid, ST);
        } else if (WIFCONTINUED(status)) {
            update_job_state(pgid, BG);
        }
    }
    errno = olderrno;
}

// SIGINT handler: forward Ctrl+C to foreground job
void sigint_handler(int sig) {
    job_t* job = get_fg_job();
    if (job) {
        kill(-job->pgid, SIGINT);
    }
}

// SIGTSTP handler: forward Ctrl+Z to foreground job
void sigtstp_handler(int sig) {
    job_t* job = get_fg_job();
    if (job) {
        kill(-job->pgid, SIGTSTP);
        update_job_state(job->pgid, ST);
    }
}

// Resume a stopped job in background
void do_bg(char **argv) {
    if (!argv[1]) { printf("bg: missing job number\n"); return; }
    char *p = argv[1];
    if (*p=='%') p++;
    int jid = atoi(p);
    job_t *job = get_job_jid(jid);
    if (!job) { printf("bg: %s: no such job\n", argv[1]); return; }
    kill(-job->pgid, SIGCONT);
    update_job_state(job->pgid, BG);
    printf("[%d] %s &\n", job->job_id, job->cmdline);
}

// Bring a job to foreground
void do_fg(char **argv) {
    if (!argv[1]) { printf("fg: missing job number\n"); return; }
    char *p = argv[1];
    if (*p=='%') p++;
    int jid = atoi(p);
    job_t *job = get_job_jid(jid);
    if (!job) { printf("fg: %s: no such job\n", argv[1]); return; }
    // continue the stopped job
    kill(-job->pgid, SIGCONT);
    // set state to foreground
    update_job_state(job->pgid, FG);

    // give terminal control to job
    tcsetpgrp(STDIN_FILENO, job->pgid);
    int stopped = 0;
    for (int i = 0; i < job->n_processes; i++) {
        int status;
        waitpid(job->pids[i], &status, WUNTRACED);
        if (WIFSTOPPED(status)) stopped = 1;
    }
    // restore terminal to shell
    tcsetpgrp(STDIN_FILENO, shell_pgid);
    if (stopped) {
        // job remained stopped, update state
        update_job_state(job->pgid, ST);
        printf("[%d] Stopped   %s", job->job_id, job->cmdline);
    } else {
        // job exited or terminated
        delete_job(job->pgid);
    }
}

// Kill a job
void do_kill(char **argv) {
    if (!argv[1]) { printf("kill: missing job number\n"); return; }
    char *p = argv[1];
    if (*p=='%') p++;
    int jid = atoi(p);
    job_t *job = get_job_jid(jid);
    if (!job) { printf("kill: %s: no such job\n", argv[1]); return; }
    kill(-job->pgid, SIGTERM);
}

// remove completed jobs
void cleanup_jobs(void) {
    // Remove completed jobs at head
    while (job_list && job_is_completed(job_list->pgid) == 1) {
        delete_job(job_list->pgid);
    }
    // Remove completed jobs beyond head
    job_t *prev = job_list;
    while (prev && prev->next) {
        job_t *curr = prev->next;
        if (job_is_completed(curr->pgid) == 1) {
            delete_job(curr->pgid);
            // prev->next updated by delete_job
        } else {
            prev = curr;
        }
    }
}
