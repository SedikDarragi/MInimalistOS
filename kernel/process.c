#include "process.h"
#include "../include/vga.h"
#include "utils.h"

// Local VGA functions for process system
static void proc_vga_print(const char* str) {
    volatile uint16_t* vga = (volatile uint16_t*)0xB8000;
    static int pos = 80 * 10; // Start at line 10
    
    while (*str) {
        if (*str == '\n') {
            pos = ((pos / 80) + 1) * 80;
            if (pos >= 80 * 25) pos = 0;
        } else {
            vga[pos++] = (0x0F << 8) | *str;
            if (pos >= 80 * 25) pos = 0;
        }
        str++;
    }
}

// Suppress unused parameter warnings
#define UNUSED(x) (void)(x)

static process_t processes[MAX_PROCESSES];
static int next_pid = 1;

void process_init(void) {
    memset(processes, 0, sizeof(processes));
    
    // Create initial kernel process
    process_create("kernel", NULL);
}

int process_create(const char* name, void (*entry_point)()) {
    UNUSED(entry_point);  // Will be used when process scheduling is implemented
    if (next_pid >= MAX_PROCESSES) {
        return -1; // No more process slots
    }
    
    process_t* p = &processes[next_pid];
    p->pid = next_pid;
    strncpy(p->name, name, MAX_PROCESS_NAME - 1);
    p->name[MAX_PROCESS_NAME - 1] = '\0';
    p->state = PROCESS_READY;
    p->priority = 1;
    p->runtime = 0;
    
    // TODO: Set up stack and context for the new process
    
    return next_pid++;
}

void process_exit(int status) {
    UNUSED(status);  // Will be used when process cleanup is implemented
    // TODO: Clean up process resources
    // For now, just mark as zombie
    process_t* current = &processes[0]; // Get current process
    current->state = PROCESS_ZOMBIE;
}

process_t* process_get(int pid) {
    if (pid < 0 || pid >= MAX_PROCESSES) {
        return NULL;
    }
    return &processes[pid];
}

void process_print_list(void) {
    proc_vga_print("  PID  STATE     RUNTIME  PRIORITY  NAME\n");
    proc_vga_print("  ---  --------  -------  --------  ----\n");
    
    for (int i = 0; i < next_pid; i++) {
        if (processes[i].pid == 0) continue;
        
        // Print PID
        char pid_str[8];
        itoa(processes[i].pid, pid_str, 10);
        proc_vga_print("  ");
        proc_vga_print(pid_str);
        proc_vga_print("  ");
        
        // Print state
        switch (processes[i].state) {
            case PROCESS_RUNNING: proc_vga_print("RUN     "); break;
            case PROCESS_READY:   proc_vga_print("READY   "); break;
            case PROCESS_BLOCKED: proc_vga_print("BLOCKED "); break;
            case PROCESS_ZOMBIE:  proc_vga_print("ZOMBIE  "); break;
            default:              proc_vga_print("?       "); break;
        }
        
        // Print runtime
        char runtime_str[16];
        itoa(processes[i].runtime, runtime_str, 10);
        proc_vga_print("  ");
        proc_vga_print(runtime_str);
        proc_vga_print("  ");
        
        // Print priority
        char priority_str[4];
        itoa(processes[i].priority, priority_str, 10);
        proc_vga_print("  ");
        proc_vga_print(priority_str);
        proc_vga_print("      ");
        
        // Print name
        proc_vga_print(processes[i].name);
        proc_vga_print("\n");
    }
}
