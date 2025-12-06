#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

// System call numbers
#define SYS_EXIT    1
#define SYS_WRITE   2
#define SYS_READ    3
#define SYS_FORK    4
#define SYS_WAIT    5
#define SYS_EXEC    6
#define SYS_GETPID  7
#define SYS_YIELD   8

// System call return values
#define SYS_SUCCESS 0
#define SYS_ERROR   -1

// System call structure
typedef struct {
    uint32_t eax;  // System call number
    uint32_t ebx;  // Argument 1
    uint32_t ecx;  // Argument 2
    uint32_t edx;  // Argument 3
    uint32_t esi;  // Argument 4
    uint32_t edi;  // Argument 5
} syscall_args_t;

// Function prototypes
void syscall_init(void);
uint32_t syscall_handler(syscall_args_t* args);

// User-level system call interface
uint32_t syscall(uint32_t num, uint32_t arg1, uint32_t arg2, uint32_t arg3);

// System call implementations
uint32_t sys_exit(uint32_t status);
uint32_t sys_write(uint32_t fd, const char* buf, uint32_t count);
uint32_t sys_read(uint32_t fd, char* buf, uint32_t count);
uint32_t sys_fork(void);
uint32_t sys_wait(uint32_t pid);
uint32_t sys_exec(const char* path);
uint32_t sys_getpid(void);
uint32_t sys_yield(void);

#endif
