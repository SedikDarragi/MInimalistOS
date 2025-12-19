#ifndef PROCESS_SIMPLE_H
#define PROCESS_SIMPLE_H

#include <stdint.h>

// Process states
#define PROCESS_STATE_UNUSED  0
#define PROCESS_STATE_RUNNING 1
#define PROCESS_STATE_STOPPED 2

// Function declarations
void process_init(void);
uint32_t process_create(void (*entry_point)(void), const char* name);
void process_exit(uint32_t pid);
uint32_t process_get_current(void);
void schedule(void);
void process_print_list(void);

#endif // PROCESS_SIMPLE_H
