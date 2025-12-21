#include "../include/syscall.h"
#include "../include/memory.h"
#include "../include/string.h"
#include "../drivers/vga.h"
#include "../kernel/log.h"
#include "../include/process_simple.h"
#include "../include/filesystem.h"

// System call interface implementation

// System call handler function pointer type
typedef int (*syscall_handler_t)(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);

// Maximum number of system calls
#define MAX_SYSCALLS 64

// System call table
static syscall_handler_t syscall_table[MAX_SYSCALLS];

// Initialize system call interface
void syscall_init(void) {
    // Initialize all system call handlers to NULL
    for (int i = 0; i < MAX_SYSCALLS; i++) {
        syscall_table[i] = NULL;
    }
    
    // Register basic system calls
    syscall_table[SYS_EXIT] = sys_exit;
    syscall_table[SYS_WRITE] = sys_write;
    syscall_table[SYS_READ] = sys_read;
    syscall_table[SYS_OPEN] = sys_open;
    syscall_table[SYS_CLOSE] = sys_close;
    syscall_table[SYS_GETPID] = sys_getpid;
    syscall_table[SYS_YIELD] = sys_yield;
    syscall_table[SYS_MALLOC] = sys_malloc;
    syscall_table[SYS_FREE] = sys_free;
    
    log_info("System call interface initialized");
    vga_print("Syscall interface: OK\n");
}

// Register a system call
int syscall_register(uint32_t syscall_num, syscall_handler_t handler) {
    if (syscall_num >= MAX_SYSCALLS) {
        log_error("System call number too high");
        return -1;
    }
    
    syscall_table[syscall_num] = handler;
    log_info("System call registered");
    return 0;
}

// System call dispatcher (called from assembly)
int syscall_dispatch(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    if (syscall_num >= MAX_SYSCALLS || syscall_table[syscall_num] == NULL) {
        log_error("Invalid system call");
        return -1;
    }
    
    // Call the system call handler
    return syscall_table[syscall_num](arg1, arg2, arg3, arg4);
}

// System call implementations

// SYS_EXIT: Exit current process
int sys_exit(uint32_t exit_code, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    (void)arg2; (void)arg3; (void)arg4;
    
    log_info("Process exit called");
    
    // Get current process and exit it
    uint32_t pid = process_get_current();
    process_exit(pid);
    
    // This should never return
    while (1) {
        __asm__ volatile("hlt");
    }
    
    return exit_code;
}

// SYS_WRITE: Write to file descriptor
int sys_write(uint32_t fd, uint32_t buffer, uint32_t count, uint32_t arg4) {
    (void)arg4;
    
    if (fd == 1 || fd == 2) { // stdout or stderr
        // Write to VGA
        char* buf = (char*)buffer;
        for (uint32_t i = 0; i < count; i++) {
            vga_putchar(buf[i]);
        }
        return count;
    } else {
        // Write to file
        return vfs_write(fd, (void*)buffer, count);
    }
}

// SYS_READ: Read from file descriptor
int sys_read(uint32_t fd, uint32_t buffer, uint32_t count, uint32_t arg4) {
    (void)arg4;
    
    if (fd == 0) { // stdin
        // Read from keyboard (simplified)
        char* buf = (char*)buffer;
        for (uint32_t i = 0; i < count; i++) {
            buf[i] = 'A'; // Simplified - return 'A' for now
        }
        return count;
    } else {
        // Read from file
        return vfs_read(fd, (void*)buffer, count);
    }
}

// SYS_OPEN: Open a file
int sys_open(uint32_t path_ptr, uint32_t flags, uint32_t arg3, uint32_t arg4) {
    (void)arg3; (void)arg4;
    
    char* path = (char*)path_ptr;
    return vfs_open(path, flags);
}

// SYS_CLOSE: Close a file
int sys_close(uint32_t fd, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    (void)arg2; (void)arg3; (void)arg4;
    
    return vfs_close(fd);
}

// SYS_GETPID: Get process ID
int sys_getpid(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    (void)arg1; (void)arg2; (void)arg3; (void)arg4;
    
    return process_get_current();
}

// SYS_YIELD: Yield CPU to another process
int sys_yield(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    (void)arg1; (void)arg2; (void)arg3; (void)arg4;
    
    schedule();
    return 0;
}

// SYS_MALLOC: Allocate memory
int sys_malloc(uint32_t size, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    (void)arg2; (void)arg3; (void)arg4;
    
    void* ptr = kmalloc(size);
    return (uint32_t)ptr;
}

// SYS_FREE: Free memory
int sys_free(uint32_t ptr, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    (void)arg2; (void)arg3; (void)arg4;
    
    kfree((void*)ptr);
    return 0;
}

// Additional system calls can be added here...

// SYS_SLEEP: Sleep for specified number of timer ticks
int sys_sleep(uint32_t ticks, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    (void)arg2; (void)arg3; (void)arg4;
    
    // Simplified sleep - just yield multiple times
    for (uint32_t i = 0; i < ticks; i++) {
        schedule();
    }
    return 0;
}

// SYS_GET_TIME: Get system time (timer ticks)
int sys_get_time(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    (void)arg1; (void)arg2; (void)arg3; (void)arg4;
    
    // Return timer ticks (simplified)
    extern volatile uint32_t timer_ticks;
    return timer_ticks;
}
