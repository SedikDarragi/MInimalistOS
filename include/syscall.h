#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

// Basic system call numbers
#define SYS_EXIT      1
#define SYS_WRITE     2
#define SYS_READ      3
#define SYS_OPEN      4
#define SYS_CLOSE     5
#define SYS_GETPID    6
#define SYS_YIELD     7
#define SYS_MALLOC    8
#define SYS_FREE      9
#define SYS_SLEEP     10
#define SYS_GET_TIME  11
#define SYS_LOG       12
#define SYS_GET_STATS 13

// System call return values
#define SYS_SUCCESS 0
#define SYS_ERROR   -1

// System call handler
void syscall_init(void);
int syscall_dispatch(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);
typedef int (*syscall_handler_t)(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);
int syscall_register(uint32_t syscall_num, syscall_handler_t handler);

// System call implementations
int sys_exit(uint32_t exit_code, uint32_t arg2, uint32_t arg3, uint32_t arg4);
int sys_write(uint32_t fd, uint32_t buffer, uint32_t count, uint32_t arg4);
int sys_read(uint32_t fd, uint32_t buffer, uint32_t count, uint32_t arg4);
int sys_open(uint32_t path_ptr, uint32_t flags, uint32_t arg3, uint32_t arg4);
int sys_close(uint32_t fd, uint32_t arg2, uint32_t arg3, uint32_t arg4);
int sys_getpid(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);
int sys_yield(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);
int sys_malloc(uint32_t size, uint32_t arg2, uint32_t arg3, uint32_t arg4);
int sys_free(uint32_t ptr, uint32_t arg2, uint32_t arg3, uint32_t arg4);
int sys_sleep(uint32_t ticks, uint32_t arg2, uint32_t arg3, uint32_t arg4);
int sys_get_time(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);
int sys_log(uint32_t level, uint32_t message, uint32_t arg3, uint32_t arg4);
int sys_get_stats(uint32_t type, uint32_t buffer, uint32_t arg3, uint32_t arg4);

#endif // SYSCALL_H
