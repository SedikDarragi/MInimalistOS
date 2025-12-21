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

// System call return values
#define SYS_SUCCESS 0
#define SYS_ERROR   -1

// System call structure
typedef struct {
    uint32_t eax;  // System call number
    uint32_t ebx;  // System call handler
    uint32_t ecx;  // Argument 1
    uint32_t edx;  // Argument 2
    uint32_t esi;  // Argument 3
    uint32_t edi;  // Argument 4
} syscall_args_t;

// Function prototypes
void syscall_init(void);
int syscall_dispatch(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);
int syscall_register(uint32_t syscall_num, void* handler);

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

// User-level system call interface
uint32_t syscall(uint32_t num, uint32_t arg1, uint32_t arg2, uint32_t arg3);

// System call implementations
uint32_t sys_write(uint32_t fd, const char* buf, uint32_t count);
uint32_t sys_read(uint32_t fd, char* buf, uint32_t count);
uint32_t sys_open(const char* filename, uint32_t mode);
uint32_t sys_close(int fd);
uint32_t sys_getpid(void);
uint32_t sys_yield(void);
uint32_t sys_malloc(uint32_t size);
uint32_t sys_free(void* ptr);
uint32_t sys_sleep(uint32_t time);
uint32_t sys_get_time(void);
uint32_t sys_device_write(const char* name, const void* buffer, uint32_t size);
uint32_t sys_device_ioctl(const char* name, uint32_t cmd, void* arg);
uint32_t sys_setuid(uint32_t uid);
uint32_t sys_setgid(uint32_t gid);
uint32_t sys_getuid(void);
uint32_t sys_getgid(void);
uint32_t sys_chmod(const char* path, uint32_t mode);
uint32_t sys_chown(const char* path, uint32_t uid, uint32_t gid);
uint32_t sys_log(uint8_t level, const char* message);
uint32_t sys_get_stats(uint32_t stats_type, void* buffer);
uint32_t sys_dump_logs(void);
uint32_t sys_power_state(uint32_t state);
uint32_t sys_get_battery_info(void* buffer);
uint32_t sys_get_power_stats(void* buffer);

##endif // SYSCALL_H
