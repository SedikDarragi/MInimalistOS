#ifndef USERMODE_H
#define USERMODE_H

#include <stdint.h>

// Privilege levels
#define RING0   0x00  // Kernel mode
#define RING1   0x20  // Driver mode
#define RING2   0x40  // Driver mode
#define RING3   0x60  // User mode

// User memory segment selectors
#define USER_CS 0x18 | RING3  // User code segment
#define USER_DS 0x20 | RING3  // User data segment

// TSS structure for task switching
typedef struct {
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed)) tss_t;

// User mode functions
void usermode_init(void);
void enter_usermode(uint32_t entry_point, uint32_t stack_top);
void setup_tss(uint32_t kernel_stack);

// User process creation
uint32_t create_user_process(void (*entry_point)());

// System call for user mode transition
void syscall_enter_usermode(void);

#endif
