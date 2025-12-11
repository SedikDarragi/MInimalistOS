#include "syscall.h"
#include "process.h"
#include "../include/idt.h"
#include "../include/vga.h"
#include "../include/filesystem.h"
#include "../include/ipc.h"
#include "../include/network.h"
#include "../include/vm.h"
#include "../include/device.h"
#include "../include/security.h"
#include "string.h"

// External current security context
extern security_context_t current_context;

// System call handler table
typedef uint32_t (*syscall_func_t)(uint32_t, uint32_t, uint32_t, uint32_t);

// Wrapper functions with correct signature
static uint32_t sys_exit_wrapper(uint32_t status, uint32_t unused2, uint32_t unused3, uint32_t unused4) {
    (void)unused2; (void)unused3; (void)unused4;
    return sys_exit(status);
}

static uint32_t sys_write_wrapper(uint32_t fd, uint32_t buf, uint32_t count, uint32_t unused4) {
    (void)unused4;
    return sys_write(fd, (const char*)buf, count);
}

static uint32_t sys_read_wrapper(uint32_t fd, uint32_t buf, uint32_t count, uint32_t unused4) {
    (void)fd; (void)buf; (void)count; (void)unused4;
    return sys_read(fd, (char*)buf, count);
}

static uint32_t sys_fork_wrapper(uint32_t unused1, uint32_t unused2, uint32_t unused3, uint32_t unused4) {
    (void)unused1; (void)unused2; (void)unused3; (void)unused4;
    return sys_fork();
}

static uint32_t sys_wait_wrapper(uint32_t pid, uint32_t unused2, uint32_t unused3, uint32_t unused4) {
    (void)pid; (void)unused2; (void)unused3; (void)unused4;
    return sys_wait(pid);
}

static uint32_t sys_exec_wrapper(uint32_t path, uint32_t unused2, uint32_t unused3, uint32_t unused4) {
    (void)path; (void)unused2; (void)unused3; (void)unused4;
    return sys_exec((const char*)path);
}

static uint32_t sys_getpid_wrapper(uint32_t unused1, uint32_t unused2, uint32_t unused3, uint32_t unused4) {
    (void)unused1; (void)unused2; (void)unused3; (void)unused4;
    return sys_getpid();
}

static uint32_t sys_yield_wrapper(uint32_t unused1, uint32_t unused2, uint32_t unused3, uint32_t unused4) {
    (void)unused1; (void)unused2; (void)unused3; (void)unused4;
    return sys_yield();
}

static uint32_t sys_open_wrapper(uint32_t filename, uint32_t mode, uint32_t unused3, uint32_t unused4) {
    (void)unused3; (void)unused4;
    return sys_open((const char*)filename, mode);
}

static uint32_t sys_close_wrapper(uint32_t fd, uint32_t unused2, uint32_t unused3, uint32_t unused4) {
    (void)unused2; (void)unused3; (void)unused4;
    return sys_close((int)fd);
}

static uint32_t sys_seek_wrapper(uint32_t fd, uint32_t position, uint32_t unused3, uint32_t unused4) {
    (void)unused3; (void)unused4;
    return sys_seek((int)fd, position);
}

static uint32_t sys_stat_wrapper(uint32_t filename, uint32_t unused2, uint32_t unused3, uint32_t unused4) {
    (void)filename; (void)unused2; (void)unused3; (void)unused4;
    return sys_stat((const char*)filename);
}

static uint32_t sys_ipc_send_wrapper(uint32_t receiver, uint32_t type, uint32_t data, uint32_t length) {
    return sys_ipc_send(receiver, (uint8_t)type, (const void*)data, (uint16_t)length);
}

static uint32_t sys_ipc_receive_wrapper(uint32_t sender, uint32_t msg, uint32_t unused3, uint32_t unused4) {
    (void)unused3; (void)unused4;
    return sys_ipc_receive(sender, (void*)msg);
}

static uint32_t sys_network_send_wrapper(uint32_t dst_ip, uint32_t type, uint32_t data, uint32_t length) {
    return sys_network_send(dst_ip, (uint8_t)type, (const void*)data, (uint16_t)length);
}

static uint32_t sys_network_receive_wrapper(uint32_t packet, uint32_t unused2, uint32_t unused3, uint32_t unused4) {
    (void)unused2; (void)unused3; (void)unused4;
    return sys_network_receive((void*)packet);
}

static uint32_t sys_vm_alloc_wrapper(uint32_t size, uint32_t flags, uint32_t unused3, uint32_t unused4) {
    (void)unused3; (void)unused4;
    return sys_vm_alloc(size, flags);
}

static uint32_t sys_vm_free_wrapper(uint32_t addr, uint32_t unused2, uint32_t unused3, uint32_t unused4) {
    (void)unused2; (void)unused3; (void)unused4;
    return sys_vm_free(addr);
}

static uint32_t sys_vm_map_wrapper(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags, uint32_t unused4) {
    (void)unused4;
    return sys_vm_map(virt_addr, phys_addr, flags);
}

static uint32_t sys_device_open_wrapper(uint32_t name, uint32_t unused2, uint32_t unused3, uint32_t unused4) {
    (void)unused2; (void)unused3; (void)unused4;
    return sys_device_open((const char*)name);
}

static uint32_t sys_device_close_wrapper(uint32_t name, uint32_t unused2, uint32_t unused3, uint32_t unused4) {
    (void)unused2; (void)unused3; (void)unused4;
    return sys_device_close((const char*)name);
}

static uint32_t sys_device_read_wrapper(uint32_t name, uint32_t buffer, uint32_t size, uint32_t unused4) {
    (void)unused4;
    return sys_device_read((const char*)name, (void*)buffer, size);
}

static uint32_t sys_device_write_wrapper(uint32_t name, uint32_t buffer, uint32_t size, uint32_t unused4) {
    (void)unused4;
    return sys_device_write((const char*)name, (const void*)buffer, size);
}

static uint32_t sys_device_ioctl_wrapper(uint32_t name, uint32_t cmd, uint32_t arg, uint32_t unused4) {
    (void)unused4;
    return sys_device_ioctl((const char*)name, cmd, (void*)arg);
}

static uint32_t sys_setuid_wrapper(uint32_t uid, uint32_t unused2, uint32_t unused3, uint32_t unused4) {
    (void)unused2; (void)unused3; (void)unused4;
    return sys_setuid(uid);
}

static uint32_t sys_setgid_wrapper(uint32_t gid, uint32_t unused2, uint32_t unused3, uint32_t unused4) {
    (void)unused2; (void)unused3; (void)unused4;
    return sys_setgid(gid);
}

static uint32_t sys_getuid_wrapper(uint32_t unused1, uint32_t unused2, uint32_t unused3, uint32_t unused4) {
    (void)unused1; (void)unused2; (void)unused3; (void)unused4;
    return sys_getuid();
}

static uint32_t sys_getgid_wrapper(uint32_t unused1, uint32_t unused2, uint32_t unused3, uint32_t unused4) {
    (void)unused1; (void)unused2; (void)unused3; (void)unused4;
    return sys_getgid();
}

static uint32_t sys_chmod_wrapper(uint32_t path, uint32_t mode, uint32_t unused3, uint32_t unused4) {
    (void)unused3; (void)unused4;
    return sys_chmod((const char*)path, mode);
}

static uint32_t sys_chown_wrapper(uint32_t path, uint32_t uid, uint32_t gid, uint32_t unused4) {
    (void)unused4;
    return sys_chown((const char*)path, uid, gid);
}

static const syscall_func_t syscall_table[] = {
    [SYS_EXIT]       = sys_exit_wrapper,
    [SYS_WRITE]      = sys_write_wrapper,
    [SYS_READ]       = sys_read_wrapper,
    [SYS_FORK]       = sys_fork_wrapper,
    [SYS_WAIT]       = sys_wait_wrapper,
    [SYS_EXEC]       = sys_exec_wrapper,
    [SYS_GETPID]     = sys_getpid_wrapper,
    [SYS_YIELD]      = sys_yield_wrapper,
    [SYS_OPEN]       = sys_open_wrapper,
    [SYS_CLOSE]      = sys_close_wrapper,
    [SYS_SEEK]       = sys_seek_wrapper,
    [SYS_STAT]       = sys_stat_wrapper,
    [SYS_IPC_SEND]   = sys_ipc_send_wrapper,
    [SYS_IPC_RECEIVE] = sys_ipc_receive_wrapper,
    [SYS_NETWORK_SEND]   = sys_network_send_wrapper,
    [SYS_NETWORK_RECEIVE] = sys_network_receive_wrapper,
    [SYS_VM_ALLOC]   = sys_vm_alloc_wrapper,
    [SYS_VM_FREE]    = sys_vm_free_wrapper,
    [SYS_VM_MAP]     = sys_vm_map_wrapper,
    [SYS_DEVICE_OPEN]   = sys_device_open_wrapper,
    [SYS_DEVICE_CLOSE]  = sys_device_close_wrapper,
    [SYS_DEVICE_READ]   = sys_device_read_wrapper,
    [SYS_DEVICE_WRITE]  = sys_device_write_wrapper,
    [SYS_DEVICE_IOCTL]  = sys_device_ioctl_wrapper,
    [SYS_SETUID]     = sys_setuid_wrapper,
    [SYS_SETGID]     = sys_setgid_wrapper,
    [SYS_GETUID]     = sys_getuid_wrapper,
    [SYS_GETGID]     = sys_getgid_wrapper,
    [SYS_CHMOD]      = sys_chmod_wrapper,
    [SYS_CHOWN]      = sys_chown_wrapper,
};

// System call interrupt handler
void syscall_interrupt_handler(struct regs* r) {
    uint32_t syscall_num = r->eax;
    
    if (syscall_num < sizeof(syscall_table) / sizeof(syscall_table[0]) && syscall_table[syscall_num]) {
        // Call the appropriate system call
        uint32_t result = syscall_table[syscall_num](r->ebx, r->ecx, r->edx, r->esi);
        r->eax = result;  // Return value in EAX
    } else {
        r->eax = SYS_ERROR;  // Invalid system call
    }
}

// Initialize system calls
void syscall_init(void) {
    // Register system call interrupt handler (int 0x80)
    register_interrupt_handler(0x80, syscall_interrupt_handler);
}

// User-level system call interface
uint32_t syscall(uint32_t num, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
    uint32_t result;
    
    asm volatile (
        "int $0x80"
        : "=a" (result)
        : "a" (num), "b" (arg1), "c" (arg2), "d" (arg3)
        : "memory"
    );
    
    return result;
}

// System call implementations
uint32_t sys_exit(uint32_t status) {
    process_exit(status);
    return SYS_SUCCESS;
}

uint32_t sys_write(uint32_t fd, const char* buf, uint32_t count) {
    if (fd == 1) {  // stdout
        volatile uint16_t* vga = (volatile uint16_t*)0xB8000;
        static int pos = 80 * 15; // Start at line 15
        
        for (uint32_t i = 0; i < count && buf[i]; i++) {
            if (buf[i] == '\n') {
                pos = ((pos / 80) + 1) * 80;
                if (pos >= 80 * 25) pos = 80 * 15;
            } else {
                vga[pos++] = 0x0F00 | buf[i];
                if (pos >= 80 * 25) pos = 80 * 15;
            }
        }
        return count;
    }
    return SYS_ERROR;
}

uint32_t sys_read(uint32_t fd, char* buf, uint32_t count) {
    (void)fd; (void)buf; (void)count; // Suppress unused parameter warnings
    // For now, just return error
    return SYS_ERROR;
}

uint32_t sys_fork(void) {
    // For now, just return current PID (simplified)
    return sys_getpid();
}

uint32_t sys_wait(uint32_t pid) {
    (void)pid; // Suppress unused parameter warning
    // For now, just return success
    return SYS_SUCCESS;
}

uint32_t sys_exec(const char* path) {
    (void)path; // Suppress unused parameter warning
    // For now, just return error
    return SYS_ERROR;
}

uint32_t sys_getpid(void) {
    process_t* current = process_get_current();
    return current ? current->pid : 0;
}

uint32_t sys_yield(void) {
    // Call scheduler to switch to another process
    schedule();
    return SYS_SUCCESS;
}

uint32_t sys_open(const char* filename, uint32_t mode) {
    int fd = fs_open(filename, mode);
    return (fd >= 0) ? fd : SYS_ERROR;
}

uint32_t sys_close(int fd) {
    int result = fs_close(fd);
    return (result == 0) ? SYS_SUCCESS : SYS_ERROR;
}

uint32_t sys_seek(int fd, uint32_t position) {
    int result = fs_seek(fd, position);
    return (result >= 0) ? result : SYS_ERROR;
}

uint32_t sys_stat(const char* filename) {
    uint32_t size = get_file_size(filename);
    return size;
}

uint32_t sys_ipc_send(uint32_t receiver, uint8_t type, const void* data, uint16_t length) {
    (void)receiver; (void)type; (void)data; (void)length;
    // Direct implementation for now
    return 0;
}

uint32_t sys_ipc_receive(uint32_t sender, void* msg) {
    (void)sender; (void)msg;
    // Direct implementation for now
    return 0;
}

uint32_t sys_vm_alloc(uint32_t size, uint32_t flags) {
    (void)size; (void)flags;
    // Direct implementation for now
    return 0x10000000;  // Return fixed address
}

uint32_t sys_vm_free(uint32_t addr) {
    (void)addr;
    return 0;
}

uint32_t sys_vm_map(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags) {
    (void)virt_addr; (void)phys_addr; (void)flags;
    return 0;
}
