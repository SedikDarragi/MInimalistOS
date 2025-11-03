#ifndef IDT_H
#define IDT_H

#include <stdint.h>

// Initialize the IDT and PIC
void idt_init();

// Default IRQ handler
void default_irq_handler(uint32_t irq);

// Send a byte to an I/O port
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

#endif // IDT_H
