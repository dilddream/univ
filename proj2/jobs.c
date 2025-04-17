#include "jobs.h"

job_t* job_list = NULL;



void init_jobs_list(void) {
    job_list = NULL;
}

int make_job_id(job_t *job) {

    if (job->next == NULL) job->job_id = 1;
    else job->job_id = job->next->job_id + 1;

    return job->job_id;
}

int add_job(pid_t pgid, int state, char* cmdline) {
    job_t* new_job = (job_t*)malloc(sizeof(job_t));
    if (new_job == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        return -1;
    }

    new_job->pgid = pgid;
    new_job->state = state;
    strcpy(new_job->cmdline, cmdline);
    new_job->next = job_list;
    job_list = new_job;

    new_job->job_id = make_job_id(new_job);

    return 0;
}

int delete_job(pid_t pgid) {
    job_t* curr = job_list;
    job_t* prev = NULL;
    
    while (curr != NULL) {
        if (curr->pgid == pgid) {
            if (prev == NULL) {
                job_list = curr->next;
            } else {
                prev->next = curr->next;
            }
            free(curr);
            return 1;
        }
        prev = curr;
        curr = curr->next;
    }
    return 0;
}

void list_jobs(void) {
    list_jobs_recursive(job_list);
}

void list_jobs_recursive(job_t* job) {
    if (job == NULL) return;

    list_jobs_recursive(job->next);

    // Print the current job
    printf("[%d]   ", job->job_id);
    switch (job->state) {
        case BG:
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

int get_job_id(pid_t pgid) {
    job_t* curr = job_list;
    
    while (curr != NULL) {
        if (curr->pgid == pgid)
            return curr->job_id;
        curr = curr->next;
    }
    return 0;
}

job_t* get_job_pgid(pid_t pgid) {
    job_t* curr = job_list;
    while (curr != NULL) {
        if (curr->pgid == pgid)
            return curr;
        curr = curr->next;
    }
    return NULL;
}

void clear_jobs(void) {
    job_t* curr = job_list;
    while (curr != NULL) {
        job_t* next = curr->next;
        free(curr->pgids);
        free(curr);
        curr = next;
    }
    job_list = NULL;
}

int add_pipeline_job(pid_t pgid, pid_t *pgids, int n_pgids, int state, char* cmdline) {
    job_t* new_job = (job_t*)malloc(sizeof(job_t));
    if (new_job == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        return -1;
    }

    new_job->pgid = pgid;
    new_job->pgids = malloc(sizeof(pid_t) * n_pgids);
    if (new_job->pgids == NULL) {
        free(new_job);
        return -1;
    }
    memcpy(new_job->pgids, pgids, sizeof(pid_t) * n_pgids);
    new_job->n_pgids = n_pgids;
    new_job->state = state;
    strcpy(new_job->cmdline, cmdline);
    new_job->next = job_list;
    job_list = new_job;

    new_job->job_id = make_job_id(new_job);

    return 0;
}