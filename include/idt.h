#ifndef IDT_H
#define IDT_H

#include <stdint.h>

// Initialize the IDT and PIC
void idt_init();

// Default IRQ handler
void default_irq_handler(uint32_t irq);

// Read a byte from an I/O port
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Send a byte to an I/O port
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Short delay for I/O operations
static inline void io_wait(void) {
    // Port 0x80 is used for 'checkpoints' during POST.
    // The Linux kernel seems to think it is free for use :-/
    asm volatile ("outb %%al, $0x80" : : "a"(0));
}

#endif // IDT_H
