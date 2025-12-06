#include "process.h"
#include "../include/vga.h"
#include "utils.h"
#include "../include/idt.h"
#include "context.h"

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
static int current_process = 0;
static int scheduler_ticks = 0;
static process_t* current_process_ptr = NULL;

// External test process functions
extern void test_process_1(void);
extern void test_process_2(void);
extern void user_process_1(void);
extern void user_process_2(void);
extern void memory_test_process(void);
extern void user_program_main(void);
extern void fs_test_process(void);

void process_init(void) {
    memset(processes, 0, sizeof(processes));
    
    // Create initial kernel process
    process_create("kernel", NULL);
    
    // Create test processes
    process_create("test1", test_process_1);
    process_create("test2", test_process_2);
    
    // Create user processes that use system calls
    process_create("user1", user_process_1);
    process_create("user2", user_process_2);
    
    // Create memory test process
    process_create("memtest", memory_test_process);
    
    // Create user mode process
    process_create("userprog", user_program_main);
    
    // Create file system test process
    process_create("fstest", fs_test_process);
    
    // Set current process to kernel process
    current_process = 0;
    current_process_ptr = &processes[0];
    current_process_ptr->state = PROCESS_RUNNING;
}

int process_create(const char* name, void (*entry_point)()) {
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
    
    // Set up process context if we have an entry point
    if (entry_point) {
        // Initialize context with stack top and entry point
        uint32_t stack_top = (uint32_t)p->stack + STACK_SIZE;
        context_init(&p->context, entry_point, stack_top);
    }
    
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

// Simple round-robin scheduler with context switching
void schedule(void) {
    scheduler_ticks++;
    
    // Update current process runtime
    if (current_process_ptr && current_process_ptr->pid != 0) {
        current_process_ptr->runtime++;
        current_process_ptr->state = PROCESS_READY;
    }
    
    // Find next ready process
    int next = current_process;
    int attempts = 0;
    
    do {
        next = (next + 1) % next_pid;
        attempts++;
        
        // Skip empty slots and zombie processes
        if (processes[next].pid != 0 && processes[next].state == PROCESS_READY) {
            process_t* old_process = current_process_ptr;
            current_process = next;
            current_process_ptr = &processes[current_process];
            current_process_ptr->state = PROCESS_RUNNING;
            
            // Show scheduler activity every 100 ticks
            if (scheduler_ticks % 100 == 0) {
                volatile uint16_t* vga = (volatile uint16_t*)0xB8000;
                vga[80*24 + 75] = 0x0F00 + ((current_process % 10) + '0');
            }
            
            // Perform context switch if we have a different process
            if (old_process != current_process_ptr && old_process && current_process_ptr) {
                context_switch(&old_process->context, &current_process_ptr->context);
            }
            
            return;
        }
    } while (attempts < next_pid);
    
    // No ready processes found, stay with current
    if (current_process_ptr && current_process_ptr->pid != 0) {
        current_process_ptr->state = PROCESS_RUNNING;
    }
}

// Get current process
process_t* process_get_current(void) {
    return current_process_ptr;
}
