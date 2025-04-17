#include "csapp.h"

/* ------------------- command_handle header ---------------------- */

/* preprocessor const */
#define READ_END 0
#define WRITE_END 1
#define MAXARGS 128
#define MAX_CMDLINE 1024
#define MAX_COMMANDS 128



/* single command structure */
typedef struct command {
    char *argv[MAXARGS];    // argv
    int argc;               // # of arg
} command_t;

/* pipeline structure */
typedef struct pipeline {
    command_t commands[MAX_COMMANDS];  // set of commands
    int n_commands;                    // # of commands
    int bg;                           // background
    pid_t pgid;                       // process group id
} pipeline_t;



/* preprocessor function */
int parseline(char *buf, char **argv);
void pipeline(char* cmdline, int bg);






/* ---------------------- execute header --------------------------- */


/* execute function */
int builtin_command(char **argv);       // built-in command
void command(char *cmdline);            // execute command







/* ------------------------ job header ----------------------------- */

/* job states ---------------------------- */
#define UNDEF 0     // undefined
#define BG 1        // background
#define ST 2        // stopped
#define FG 3        // foreground


/* job structure ------------------------- */
typedef struct job_t {
    pid_t pgid;              // 프로세스 그룹 ID

    pid_t *pids;             // 파이프라인 각 프로세스의 PID 저장 배열
    int n_processes;         // 파이프라인 프로세스 개수

    int job_id;              // 작업 번호
    int state;               // 작업 상태
    char cmdline[MAXLINE];   // 작업 명령어
    
    struct job_t *next;      // 연결 리스트
} job_t;

extern job_t *job_list;


/* job function -------------------------- */

/* set job list */
void init_job_list(void);      /* job list initialize */
void clear_job(void);          /* job list clear */

/* control job list */
int add_job(pid_t pgid, pid_t *pids, int n_processes, int state, char *cmdline);    /* add job */
int delete_job(pid_t pgid);                                                         /* delete job */

/* search job */
job_t* get_job_pgid(pid_t pgid);    /* search job by pgid (for child process) */
job_t* get_job_jid(int job_id);     /* search job by job_id */

/* control job state */
int update_job_state(pid_t pgid, int state);       /* update job state */
int job_is_completed(pid_t pgid);                   /* check if job is completed */
int job_is_stopped(pid_t pgid);                     /* check if job is stopped */

/* list job */
void list_job(job_t* job);                           /* list all jobs */



