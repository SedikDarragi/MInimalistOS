#ifndef CONTEXT_H
#define CONTEXT_H

#include <stdint.h>

// CPU context structure for process switching
typedef struct {
    uint32_t eax, ebx, ecx, edx;
    uint32_t esi, edi;
    uint32_t ebp, esp;
    uint32_t eip;
    uint32_t eflags;
    uint32_t cr3;  // Page directory
} cpu_context_t;

// Function prototypes
void context_switch(cpu_context_t* old_context, cpu_context_t* new_context);
void context_init(cpu_context_t* context, void (*entry_point)(), uint32_t stack_top);

#endif
