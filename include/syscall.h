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
#define SYS_OPEN    9
#define SYS_CLOSE   10
#define SYS_SEEK    11
#define SYS_STAT    12
#define SYS_IPC_SEND    13
#define SYS_IPC_RECEIVE 14
#define SYS_NETWORK_SEND  15
#define SYS_NETWORK_RECEIVE 16
#define SYS_VM_ALLOC    17
#define SYS_VM_FREE     18
#define SYS_VM_MAP      19

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
uint32_t sys_open(const char* filename, uint32_t mode);
uint32_t sys_close(int fd);
uint32_t sys_seek(int fd, uint32_t position);
uint32_t sys_stat(const char* filename);
uint32_t sys_ipc_send(uint32_t receiver, uint8_t type, const void* data, uint16_t length);
uint32_t sys_ipc_receive(uint32_t sender, void* msg);
uint32_t sys_network_send(uint32_t dst_ip, uint8_t type, const void* data, uint16_t length);
uint32_t sys_network_receive(void* packet);
uint32_t sys_vm_alloc(uint32_t size, uint32_t flags);
uint32_t sys_vm_free(uint32_t addr);
uint32_t sys_vm_map(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);

#endif
