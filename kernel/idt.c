#include <stddef.h> // For memset
#include "idt.h"

// PIC I/O Ports
#define PIC1_CMD    0x20
#define PIC1_DATA   0x21
#define PIC2_CMD    0xA0
#define PIC2_DATA   0xA1

// IDT entry structure
struct idt_entry {
    uint16_t base_lo;
    uint16_t sel;        // Kernel segment selector
    uint8_t  always0;
    uint8_t  flags;      // Flags
    uint16_t base_hi;
} __attribute__((packed));

// IDT pointer structure
struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct idt_entry idt[256];
struct idt_ptr   idtp;

// External assembly ISR/IRQ handlers
extern void isr0(); extern void isr1(); extern void isr2(); extern void isr3();
extern void isr4(); extern void isr5(); extern void isr6(); extern void isr7();
extern void isr8(); extern void isr9(); extern void isr10(); extern void isr11();
extern void isr12(); extern void isr13(); extern void isr14(); extern void isr15();
extern void irq0(); extern void irq1(); extern void irq2(); extern void irq3();
extern void irq4(); extern void irq5(); extern void irq6(); extern void irq7();
extern void irq8(); extern void irq9(); extern void irq10(); extern void irq11();
extern void irq12(); extern void irq13(); extern void irq14(); extern void irq15();

// Function to set a gate in the IDT
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_lo = (base & 0xFFFF);
    idt[num].base_hi = (base >> 16) & 0xFFFF;
    idt[num].sel     = sel;
    idt[num].always0 = 0;
    idt[num].flags   = flags;
}

// Remap the PIC
void pic_remap(void) {
    outb(PIC1_CMD, 0x11);
    outb(PIC2_CMD, 0x11);
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);
    outb(PIC1_DATA, 0x0);
    outb(PIC2_DATA, 0x0);
}

// Generic C-level interrupt handler
void fault_handler(struct regs *r) {
    // For now, just hang on any CPU exception
    (void)r; // Suppress unused variable warning
    asm volatile ("cli; hlt");
}

// Generic C-level IRQ handler
void irq_handler(struct regs *r) {
    // Send EOI to the PICs
    if (r->int_no >= 40) {
        outb(PIC2_CMD, 0x20);
    }
    outb(PIC1_CMD, 0x20);
}

// Main IDT initialization
void idt_init() {
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base  = (uint32_t)&idt;

    memset(&idt, 0, sizeof(struct idt_entry) * 256);

    pic_remap();

    // Set up ISRs (CPU exceptions 0-31)
    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);
    // ... (set up all 32 ISRs here)
    idt_set_gate(31, (uint32_t)isr0, 0x08, 0x8E); // Placeholder

    // Set up IRQs (hardware interrupts 32-47)
    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E);
    // ... (set up all 16 IRQs here)
    idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E);

    // Load the IDT
    asm volatile ("lidt %0" : : "m"(idtp));
    asm volatile ("sti"); // Enable interrupts
}
