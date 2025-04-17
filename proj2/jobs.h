#ifndef __JOBS_H__
#define __JOBS_H__

#include "csapp.h"

/* Job states */
#define UNDEF 0
#define BG 1
#define ST 2

/* Job struct */
typedef struct job_t {
    pid_t pgid;
    pid_t *pgids;
    int n_pgids;
    int job_id;     
    int state;           
    char cmdline[MAXLINE];   /* Command line */
    struct job_t *next;     /* Next job in the list */
} job_t;

/* Job list */
extern job_t *job_list;

/* Job functions */
void init_jobs_list(void);
int make_job_id(job_t *job);
int add_job(pid_t pgid, int state, char* cmdline);
int delete_job(pid_t pgid);
void list_jobs(void);
void list_jobs_recursive(job_t *job);
int get_job_id(pid_t pgid);
job_t* get_job_pgid(pid_t pgid);
void clear_jobs(void);
int add_pipeline_job(pid_t pgid, pid_t *pgids, int n_pgids, int state, char* cmdline);

#endif