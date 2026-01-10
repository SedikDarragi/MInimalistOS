#include "../include/syscall.h"
#include "../include/memory.h"
#include "../include/string.h"
#include "../drivers/vga.h"
#include "../kernel/log.h"
#include "../include/process_simple.h"
#include "../include/filesystem.h"

// System call interface implementation

// System call handler function pointer type (4 parameters for interrupt handler)
typedef uint32_t (*syscall_handler_t)(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);

// Maximum number of system calls
#define MAX_SYSCALLS 64

// System call table
static syscall_handler_t syscall_table[MAX_SYSCALLS];

// Wrapper functions to convert 4-parameter calls to actual syscall signatures
static uint32_t sys_exit_wrapper(uint32_t status, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    (void)arg2; (void)arg3; (void)arg4;
    return sys_exit(status);
}

static uint32_t sys_write_wrapper(uint32_t fd, uint32_t buf, uint32_t count, uint32_t arg4) {
    (void)arg4;
    return sys_write(fd, (const char*)buf, count);
}

static uint32_t sys_read_wrapper(uint32_t fd, uint32_t buf, uint32_t count, uint32_t arg4) {
    (void)arg4;
    return sys_read(fd, (char*)buf, count);
}

static uint32_t sys_open_wrapper(uint32_t path, uint32_t flags, uint32_t arg3, uint32_t arg4) {
    (void)arg3; (void)arg4;
    return sys_open((const char*)path, flags);
}

static uint32_t sys_close_wrapper(uint32_t fd, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    (void)arg2; (void)arg3; (void)arg4;
    return sys_close(fd);
}

static uint32_t sys_getpid_wrapper(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    (void)arg1; (void)arg2; (void)arg3; (void)arg4;
    return sys_getpid();
}

static uint32_t sys_yield_wrapper(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    (void)arg1; (void)arg2; (void)arg3; (void)arg4;
    return sys_yield();
}

static uint32_t sys_malloc_wrapper(uint32_t size, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    (void)arg2; (void)arg3; (void)arg4;
    return sys_malloc(size);
}

static uint32_t sys_free_wrapper(uint32_t ptr, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    (void)arg2; (void)arg3; (void)arg4;
    return sys_free(ptr);
}

static uint32_t sys_log_wrapper(uint32_t level, uint32_t message, uint32_t arg3, uint32_t arg4) {
    (void)level; (void)arg3; (void)arg4;
    // Stub implementation - just print to VGA
    const char* msg = (const char*)message;
    if (msg) {
        vga_print("[SYSLOG] ");
        vga_print(msg);
        vga_print("\n");
    }
    return 0;
}

static uint32_t sys_get_stats_wrapper(uint32_t stats_type, uint32_t buffer, uint32_t arg3, uint32_t arg4) {
    (void)stats_type; (void)buffer; (void)arg3; (void)arg4;
    return 0;  // Stub
}

static uint32_t sys_dump_logs_wrapper(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    (void)arg1; (void)arg2; (void)arg3; (void)arg4;
    return 0;  // Stub
}

static uint32_t sys_power_state_wrapper(uint32_t state, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    (void)state; (void)arg2; (void)arg3; (void)arg4;
    return 0;  // Stub
}

static uint32_t sys_get_battery_info_wrapper(uint32_t buffer, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    (void)buffer; (void)arg2; (void)arg3; (void)arg4;
    return 0;  // Stub
}

static uint32_t sys_get_power_stats_wrapper(uint32_t buffer, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    (void)buffer; (void)arg2; (void)arg3; (void)arg4;
    return 0;  // Stub
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

static uint32_t sys_network_send_wrapper(uint32_t dst_ip, uint32_t type, uint32_t data, uint32_t length) {
    return sys_network_send(dst_ip, (uint8_t)type, (const void*)data, (uint16_t)length);
}

static uint32_t sys_network_receive_wrapper(uint32_t packet, uint32_t unused2, uint32_t unused3, uint32_t unused4) {
    (void)unused2; (void)unused3; (void)unused4;
    return sys_network_receive((void*)packet);
}

// Initialize system call interface
void syscall_init(void) {
    // Initialize all system call handlers to NULL
    for (int i = 0; i < MAX_SYSCALLS; i++) {
        syscall_table[i] = NULL;
    }
    
    // Register basic system calls
    syscall_table[SYS_EXIT] = sys_exit_wrapper;
    syscall_table[SYS_WRITE] = sys_write_wrapper;
    syscall_table[SYS_READ] = sys_read_wrapper;
    syscall_table[SYS_OPEN] = sys_open_wrapper;
    syscall_table[SYS_CLOSE] = sys_close_wrapper;
    syscall_table[SYS_GETPID] = sys_getpid_wrapper;
    syscall_table[SYS_YIELD] = sys_yield_wrapper;
    syscall_table[SYS_MALLOC] = sys_malloc_wrapper;
    syscall_table[SYS_FREE] = sys_free_wrapper;
    syscall_table[SYS_LOG] = sys_log_wrapper;
    syscall_table[SYS_GET_STATS] = sys_get_stats_wrapper;
    syscall_table[SYS_DUMP_LOGS] = sys_dump_logs_wrapper;
    syscall_table[SYS_POWER_STATE] = sys_power_state_wrapper;
    syscall_table[SYS_GET_BATTERY_INFO] = sys_get_battery_info_wrapper;
    syscall_table[SYS_GET_POWER_STATS] = sys_get_power_stats_wrapper;
    syscall_table[SYS_DEVICE_OPEN] = sys_device_open_wrapper;
    syscall_table[SYS_DEVICE_CLOSE] = sys_device_close_wrapper;
    syscall_table[SYS_DEVICE_READ] = sys_device_read_wrapper;
    syscall_table[SYS_DEVICE_WRITE] = sys_device_write_wrapper;
    syscall_table[SYS_DEVICE_IOCTL] = sys_device_ioctl_wrapper;
    syscall_table[SYS_NETWORK_SEND] = sys_network_send_wrapper;
    syscall_table[SYS_NETWORK_RECEIVE] = sys_network_receive_wrapper;
    
    log_info("System call interface initialized");
    vga_print("Syscall interface: OK\n");
}

// Register a system call
uint32_t syscall_register(uint32_t syscall_num, syscall_handler_t handler) {
    if (syscall_num >= MAX_SYSCALLS) {
        log_error("System call number too high");
        return 0xFFFFFFFF;
    }
    
    syscall_table[syscall_num] = handler;
    log_info("System call registered");
    return 0;
}

// System call dispatcher (called from assembly)
uint32_t syscall_dispatch(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    if (syscall_num >= MAX_SYSCALLS || syscall_table[syscall_num] == NULL) {
        log_error("Invalid system call");
        return 0xFFFFFFFF;  // Error value
    }
    
    // Call the system call handler
    return syscall_table[syscall_num](arg1, arg2, arg3, arg4);
}

// System call implementations (actual implementations with correct signatures)

uint32_t sys_exit(uint32_t status) {
    log_info("Process exit called");
    // For now, just halt
    while (1) {
        __asm__ volatile("hlt");
    }
    return status;
}

uint32_t sys_write(uint32_t fd, const char* buf, uint32_t count) {
    if (fd == 1 || fd == 2) { // stdout or stderr
        for (uint32_t i = 0; i < count && buf[i]; i++) {
            vga_putchar(buf[i]);
        }
        return count;
    }
    return count;
}

uint32_t sys_read(uint32_t fd, char* buf, uint32_t count) {
    (void)fd; (void)buf; (void)count;
    return 0;
}

uint32_t sys_open(const char* path, uint32_t flags) {
    (void)path; (void)flags;
    return 0;
}

uint32_t sys_close(uint32_t fd) {
    (void)fd;
    return 0;
}

uint32_t sys_getpid(void) {
    return 1;
}

uint32_t sys_yield(void) {
    return 0;
}

uint32_t sys_malloc(uint32_t size) {
    return (uint32_t)kmalloc(size);
}

uint32_t sys_free(uint32_t ptr) {
    kfree((void*)ptr);
    return 0;
}

uint32_t sys_sleep(uint32_t ticks) {
    (void)ticks;
    return 0;
}

uint32_t sys_get_time(void) {
    extern volatile uint32_t timer_ticks;
    return timer_ticks;
}

// Note: sys_log, sys_get_stats, sys_power_state, sys_get_battery_info, sys_get_power_stats
// are implemented in monitor.c and power.c

// User-level system call interface
uint32_t syscall(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
    uint32_t result;
    
    asm volatile (
        "int $0x80"
        : "=a" (result)
        : "a" (syscall_num), "b" (arg1), "c" (arg2), "d" (arg3)
        : "memory"
    );
    
    return result;
}
