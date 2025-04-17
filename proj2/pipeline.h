#include "csapp.h"
#define READ_END 0
#define WRITE_END 1

#define INITIAL_SIZE 4

/* pipeline group struct */
typedef struct {
    pid_t *pgids;     // pgid 배열
    int size;        // 현재 배열 크기
    int count;       // 현재 저장된 pgid 개수
} pgid_array_t;

/* pgid functions */
void init_pgid_array(pgid_array_t *arr);
void add_pgid(pgid_array_t *arr, pid_t pgid);
void free_pgid_array(pgid_array_t *arr);

/* pipeline main function */
void pipeline(char* cmdline);