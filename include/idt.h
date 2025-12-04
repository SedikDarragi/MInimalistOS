#ifndef IDT_H
#define IDT_H

#include <stdint.h>

/* This defines what the stack looks like after an ISR was running */
struct regs {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
};

// Function Prototypes
void idt_init(void);
void register_interrupt_handler(uint8_t n, void (*handler)(struct regs*));
void enable_irq(uint8_t irq);
void irq_handler(struct regs* r);

#endif
