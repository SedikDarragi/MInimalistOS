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
    "    push %ebp\n"              // Set up stack frame
    "    mov %esp, %ebp\n"
    "    push %eax\n"              // Save original EAX on stack
    "    mov 8(%ebp), %eax\n"      // Get old_context pointer (skip EBP and Return Addr)
    "    test %eax, %eax\n"        // Check if NULL
    "    jz 2f\n"                  // If NULL, skip saving
    "    pop %edx\n"               // Get original EAX from stack
    "    mov %edx, 0(%eax)\n"      // Save original EAX
    "    mov %ebx, 4(%eax)\n"      // Save EBX
    "    mov %ecx, 8(%eax)\n"      // Save ECX
    "    mov %edx, 12(%eax)\n"     // Save EDX
    "    mov %esi, 16(%eax)\n"     // Save ESI
    "    mov %edi, 20(%eax)\n"     // Save EDI
    "    mov %ebp, 24(%eax)\n"     // Save EBP
    "    mov %esp, 28(%eax)\n"     // Save ESP
    "    mov 4(%ebp), %edx\n"      // Get return address (EIP)
    "    mov %edx, 32(%eax)\n"     // Save EIP
    "    pushfl\n"
    "    pop %edx\n"
    "    mov %edx, 36(%eax)\n"     // Save EFLAGS
    "    jmp 3f\n"
    "    2:\n"
    "    pop %eax\n"               // Clean up stack if we skipped saving
    "    3:\n"
    "    mov 12(%ebp), %eax\n"     // Get new_context pointer
    "    mov 36(%eax), %edx\n"     // Get new EFLAGS
    "    push %edx\n"
    "    popfl\n"                  // Load EFLAGS
    "    mov 28(%eax), %esp\n"     // Load ESP
    "    push 32(%eax)\n"          // Push new EIP to simulate return address
    "    mov 24(%eax), %ebp\n"     // Load EBP
    "    mov 20(%eax), %edi\n"     // Load EDI
    "    mov 16(%eax), %esi\n"     // Load ESI
    "    mov 12(%eax), %edx\n"     // Load EDX
    "    mov 8(%eax), %ecx\n"      // Load ECX
    "    mov 4(%eax), %ebx\n"      // Load EBX
    "    mov 0(%eax), %eax\n"      // Load EAX
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
