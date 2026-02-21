#include "context.h"
#include "process.h"
#include "../include/idt.h"

// External assembly functionszeze
extern void context_switch(cpu_context_t* old_context, cpu_context_t* new_context);

// Implementation of context_switch using inline assembly (GAS syntax)
// This avoids needing a separate .asm file and Makefile changes
__asm__(
    ".global context_switch\n"
    "context_switch:\n"
    "    mov 4(%esp), %eax\n"      // Get old_context pointer
    "    test %eax, %eax\n"        // Check if NULL
    "    jz 2f\n"                  // If NULL, skip saving
    "    mov %eax, 0(%eax)\n"      // Save EAX
    "    mov %ebx, 4(%eax)\n"      // Save EBX
    "    mov %ecx, 8(%eax)\n"      // Save ECX
    "    mov %edx, 12(%eax)\n"     // Save EDX
    "    mov %esi, 16(%eax)\n"     // Save ESI
    "    mov %edi, 20(%eax)\n"     // Save EDI
    "    mov %ebp, 24(%eax)\n"     // Save EBP
    "    mov %esp, 28(%eax)\n"     // Save ESP
    "    mov (%esp), %edx\n"       // Get return address
    "    mov %edx, 32(%eax)\n"     // Save EIP
    "    pushfl\n"
    "    pop %edx\n"
    "    mov %edx, 36(%eax)\n"     // Save EFLAGS
    "    mov %cr3, %edx\n"
    "    mov %edx, 40(%eax)\n"     // Save CR3
    
    "2:\n"
    "    mov 8(%esp), %eax\n"      // Get new_context pointer
    "    mov 40(%eax), %edx\n"     // Get new CR3
    "    mov %cr3, %ecx\n"
    "    cmp %ecx, %edx\n"
    "    je 3f\n"
    "    mov %edx, %cr3\n"         // Load CR3
    "3:\n"
    "    mov 36(%eax), %edx\n"     // Get new EFLAGS
    "    push %edx\n"
    "    popfl\n"                  // Load EFLAGS
    "    mov 28(%eax), %esp\n"     // Load ESP
    "    mov 32(%eax), %edx\n"     // Get new EIP
    "    mov %edx, (%esp)\n"       // Setup return address
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
    // Treat context as an array of uint32_t for initialization
    // Layout matches the assembly offsets:
    // 0:EAX, 1:EBX, 2:ECX, 3:EDX, 4:ESI, 5:EDI, 6:EBP, 7:ESP, 8:EIP, 9:EFLAGS, 10:CR3
    uint32_t* ctx = (uint32_t*)context;
    
    // Initialize registers to 0
    for (int i = 0; i < 7; i++) ctx[i] = 0;
    
    ctx[7] = stack_top - 4;      // ESP (simulating pushed return address)
    ctx[8] = (uint32_t)entry_point; // EIP
    ctx[9] = 0x202;              // EFLAGS (Interrupts enabled)
    
    uint32_t cr3;
    asm volatile("mov %%cr3, %0" : "=r"(cr3));
    ctx[10] = cr3;               // CR3
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
