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
#define SYS_FORK      14
#define SYS_WAIT      15
#define SYS_EXEC      16
#define SYS_SEEK      17
#define SYS_STAT      18
#define SYS_NETWORK_SEND  19
#define SYS_NETWORK_RECEIVE 20
#define SYS_DEVICE_OPEN  21
#define SYS_DEVICE_CLOSE 22
#define SYS_DEVICE_READ  23
#define SYS_DEVICE_WRITE 24
#define SYS_DEVICE_IOCTL 25
#define SYS_SETUID    26
#define SYS_SETGID    27
#define SYS_GETUID    28
#define SYS_GETGID    29
#define SYS_CHMOD     30
#define SYS_CHOWN     31
#define SYS_DUMP_LOGS 32
#define SYS_POWER_STATE 33
#define SYS_GET_BATTERY_INFO 34
#define SYS_GET_POWER_STATS 35

// System call return values
#define SYS_SUCCESS 0
#define SYS_ERROR   -1

// System call handler
void syscall_init(void);
int syscall_dispatch(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);
typedef int (*syscall_handler_t)(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);
int syscall_register(uint32_t syscall_num, syscall_handler_t handler);

// C wrapper for invoking a system call from C code
uint32_t syscall(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3);

// System call implementations
uint32_t sys_exit(uint32_t status);
uint32_t sys_write(uint32_t fd, const char* buf, uint32_t count);
uint32_t sys_read(uint32_t fd, char* buf, uint32_t count);
uint32_t sys_open(const char* path, uint32_t flags);
uint32_t sys_close(uint32_t fd);
uint32_t sys_getpid(void);
uint32_t sys_yield(void);
uint32_t sys_malloc(uint32_t size);
uint32_t sys_free(uint32_t ptr);
uint32_t sys_sleep(uint32_t ticks);
uint32_t sys_get_time(void);
uint32_t sys_log(uint8_t level, const char* message);
uint32_t sys_get_stats(uint32_t stats_type, void* buffer);
uint32_t sys_fork(void);
uint32_t sys_wait(uint32_t pid);
uint32_t sys_exec(const char* path);
uint32_t sys_seek(int fd, uint32_t position);
uint32_t sys_stat(const char* filename);
uint32_t sys_network_send(uint32_t dst_ip, uint32_t type, const void* data, uint32_t length);
uint32_t sys_network_receive(void* buffer, uint32_t size);
uint32_t sys_device_open(const char* name);
uint32_t sys_device_close(uint32_t fd);
uint32_t sys_device_read(const char* name, void* buffer, uint32_t size);
uint32_t sys_device_write(const char* name, const void* buffer, uint32_t size);
uint32_t sys_device_ioctl(const char* name, uint32_t cmd, void* arg);
uint32_t sys_setuid(uint32_t uid);
uint32_t sys_setgid(uint32_t gid);
uint32_t sys_getuid(void);
uint32_t sys_getgid(void);
uint32_t sys_chmod(const char* path, uint32_t mode);
uint32_t sys_chown(const char* path, uint32_t uid, uint32_t gid);
uint32_t sys_dump_logs(void);
uint32_t sys_power_state(void);
uint32_t sys_get_battery_info(void* buffer);
uint32_t sys_get_power_stats(void* buffer);

#endif // SYSCALL_H
