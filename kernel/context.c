#include "context.h"
#include "process.h"
#include "../include/idt.h"

// External assembly functionszeze
extern void context_switch(cpu_context_t* old_context, cpu_context_t* new_context);
extern void context_init(cpu_context_t* context, void (*entry_point)(), uint32_t stack_top);

// Current running context
static cpu_context_t* current_context = NULL;

// Switch to a new process context
void switch_to_process(process_t* new_process) {
    if (!new_process) return;
    
    cpu_context_t* new_context = &new_process->context;
    
    if (current_context) {
        // Save current context and load new one
        context_switch(current_context, new_context);
    } else {
        // First switch - just load new context
        current_context = new_context;
        
        // Jump to new context
        asm volatile (
            "movl %0, %%esp\n"
            "popl %%eax\n"
            "popl %%ebx\n"
            "popl %%ecx\n"
            "popl %%edx\n"
            "popl %%esi\n"
            "popl %%edi\n"
            "popl %%ebp\n"
            "ret\n"
            : : "r" (new_context) : "memory"
        );
    }
    
    current_context = new_context;
}

// Initialize a process context
void setup_process_context(process_t* process, void (*entry_point)()) {
    // Allocate stack (4KB for now)
    uint32_t stack_top = (uint32_t)process->stack + STACK_SIZE;
    
    // Initialize context
    context_init(&process->context, entry_point, stack_top);
}
