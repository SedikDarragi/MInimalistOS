#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

#define MAX_PROCESSES 32
#define MAX_PROCESS_NAME 32

typedef enum {
    PROCESS_RUNNING,
    PROCESS_READY,
    PROCESS_BLOCKED,
    PROCESS_ZOMBIE
} process_state_t;

typedef struct {
    int pid;
    char name[MAX_PROCESS_NAME];
    process_state_t state;
    uint32_t priority;
    uint32_t runtime;
    void* stack_ptr;
} process_t;

void process_init(void);
int process_create(const char* name, void (*entry_point)());
void process_exit(int status);
process_t* process_get(int pid);
void process_print_list(void);

#endif
