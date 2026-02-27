#include "context.h"
#include "process.h"
#include "../include/idt.h"
#include <stddef.h>
#include "string.h"

// External assembly functions
void context_switch(cpu_context_t* old_context, cpu_context_t* new_context);
void context_init(cpu_context_t* context, void (*entry_point)(), uint32_t stack_top);

__asm__(
    ".global context_switch\n"
    "context_switch:\n"
    "    mov 4(%esp), %eax      # old_context\n"
    "    mov 8(%esp), %edx      # new_context\n"
    "\n"
    "    # Save registers to old_context\n"
    "    mov %ebx, 4(%eax)\n"
    "    mov %ecx, 8(%eax)\n"
    "    mov %esi, 16(%eax)\n"
    "    mov %edi, 20(%eax)\n"
    "    mov %ebp, 24(%eax)\n"
    "    mov %esp, 28(%eax)\n"
    "    pushf\n"
    "    popl 36(%eax)\n"
    "    movl (%esp), %ecx\n"
    "    movl %ecx, 32(%eax)\n"
    "\n"
    "    # Load registers from new_context\n"
    "    mov 28(%edx), %esp\n"
    "    mov 4(%edx), %ebx\n"
    "    mov 8(%edx), %ecx\n"
    "    mov 16(%edx), %esi\n"
    "    mov 20(%edx), %edi\n"
    "    mov 24(%edx), %ebp\n"
    "    pushl 36(%edx)\n"
    "    popf\n"
    "    pushl 32(%edx)\n"
    "    # eax and edx are caller-saved and not restored from context\n"
    "    ret\n"
);

void context_init(cpu_context_t* context, void (*entry_point)(), uint32_t stack_top) {
    memset(context, 0, sizeof(cpu_context_t));
    context->eip = (uint32_t)entry_point;
    context->esp = stack_top;
    context->eflags = 0x202; // Interrupts enabled
}
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
