#include "../include/memory.h"
#include "../drivers/vga.h"
#include "../kernel/log.h"
#include "../include/string.h"
#include "../include/process_simple.h"

// Simple process management system
#define MAX_PROCESSES 16
#define KERNEL_STACK_SIZE 4096

typedef struct {
    uint32_t pid;
    uint32_t state;
    uint32_t *stack;
    void (*entry_point)(void);
    uint32_t esp;
    uint32_t ebp;
    char name[32];
} simple_process_t;

static simple_process_t processes[MAX_PROCESSES];
static uint32_t current_process = 0;
static uint32_t next_pid = 1;

void process_init(void) {
    // Initialize all processes as unused
    for (int i = 0; i < MAX_PROCESSES; i++) {
        processes[i].pid = 0;
        processes[i].state = 0;
        processes[i].stack = NULL;
        processes[i].entry_point = NULL;
        processes[i].esp = 0;
        processes[i].ebp = 0;
        processes[i].name[0] = '\0';
    }
    
    // Create kernel process (PID 1)
    processes[0].pid = next_pid++;
    processes[0].state = 1; // RUNNING
    processes[0].stack = NULL; // Uses kernel stack
    processes[0].entry_point = NULL;
    processes[0].esp = 0;
    processes[0].ebp = 0;
    strcpy(processes[0].name, "kernel");
    
    log_info("Process management initialized");
    vga_print("Process management: OK\n");
}

uint32_t process_create(void (*entry_point)(void), const char* name) {
    // Find free process slot
    int slot = -1;
    for (int i = 1; i < MAX_PROCESSES; i++) {
        if (processes[i].pid == 0) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        log_error("No free process slots");
        return 0;
    }
    
    // Allocate stack for process
    uint32_t *stack = kmalloc(KERNEL_STACK_SIZE);
    if (!stack) {
        log_error("Failed to allocate process stack");
        return 0;
    }
    
    // Initialize process
    processes[slot].pid = next_pid++;
    processes[slot].state = 1; // RUNNING
    processes[slot].stack = stack;
    processes[slot].entry_point = entry_point;
    processes[slot].esp = (uint32_t)stack + KERNEL_STACK_SIZE - 16;
    processes[slot].ebp = processes[slot].esp;
    
    if (name) {
        strncpy(processes[slot].name, name, 31);
        processes[slot].name[31] = '\0';
    } else {
        strcpy(processes[slot].name, "unknown");
    }
    
    // Set up initial stack frame
    uint32_t *stack_ptr = (uint32_t*)processes[slot].esp;
    stack_ptr[0] = (uint32_t)entry_point; // Return address
    stack_ptr[1] = 0; // EFLAGS
    stack_ptr[2] = 0; // EAX
    stack_ptr[3] = 0; // ECX
    stack_ptr[4] = 0; // EDX
    
    log_info("Created new process");
    return processes[slot].pid;
}

void process_exit(uint32_t pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].pid == pid) {
            if (processes[i].stack) {
                kfree(processes[i].stack);
            }
            processes[i].pid = 0;
            processes[i].state = 0;
            processes[i].stack = NULL;
            processes[i].entry_point = NULL;
            processes[i].esp = 0;
            processes[i].ebp = 0;
            processes[i].name[0] = '\0';
            
            log_info("Process exited");
            return;
        }
    }
}

uint32_t process_get_current(void) {
    return processes[current_process].pid;
}

void schedule(void) {
    // Simple round-robin scheduler
    uint32_t next = current_process;
    
    // Find next runnable process
    for (int i = 1; i <= MAX_PROCESSES; i++) {
        uint32_t candidate = (current_process + i) % MAX_PROCESSES;
        if (processes[candidate].pid != 0 && processes[candidate].state == 1) {
            next = candidate;
            break;
        }
    }
    
    if (next != current_process) {
        // Switch processes (simplified - just update current)
        current_process = next;
        
        log_debug("Process switch occurred");
    }
}

void process_print_list(void) {
    vga_print("\n=== Process List ===\n");
    vga_print("PID  State  Name\n");
    vga_print("---  -----  ----\n");
    
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].pid != 0) {
            char buffer[64];
            // Simple string formatting without snprintf
            strcpy(buffer, "PID ");
            // Convert PID to string (simplified)
            if (processes[i].pid < 10) {
                buffer[4] = processes[i].pid + '0';
                buffer[5] = ' ';
                buffer[6] = '\0';
            } else {
                buffer[4] = (processes[i].pid / 10) + '0';
                buffer[5] = (processes[i].pid % 10) + '0';
                buffer[6] = ' ';
                buffer[7] = '\0';
            }
            strcat(buffer, processes[i].state == 1 ? "RUN  " : "STOP ");
            strcat(buffer, processes[i].name);
            strcat(buffer, "\n");
            vga_print(buffer);
        }
    }
    vga_print("===================\n");
}
