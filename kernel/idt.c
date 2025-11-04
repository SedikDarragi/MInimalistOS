#include <stdint.h>
#include "vga.h"  // For vga_printf
#include "idt.h"  // For IDT-related declarations

// IDT entry structure
struct idt_entry {
    uint16_t base_low;      // The lower 16 bits of the ISR's address
    uint16_t selector;      // Kernel segment selector
    uint8_t  always0;       // Must be zero
    uint8_t  flags;         // Type and attributes
    uint16_t base_high;     // The upper 16 bits of the ISR's address
} __attribute__((packed));

// IDT pointer structure
struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

// IDT with 256 entries
static struct idt_entry idt[256];
static struct idt_ptr idtp;

// External function to load IDT
extern void idt_load(uint32_t);

// External assembly IRQ handlers
extern void irq0();

// Default exception handler
static void default_exception_handler() {
    // Just halt the CPU for now
    asm volatile ("hlt");
}

// Simple VGA text output function for debugging
static void debug_putchar(char c) {
    static uint16_t* vga_buffer = (uint16_t*)0xB8000;
    static size_t vga_index = 0;
    
    if (c == '\n') {
        vga_index = (vga_index + 80) & ~0x7F;
    } else {
        vga_buffer[vga_index++] = (0x0F << 8) | c;
    }
    
    // Simple newline handling
    if (vga_index >= 80 * 25) {
        vga_index = 0;
    }
}

// Default IRQ handler
void default_irq_handler(uint32_t irq) {
    // Debug output
    debug_putchar('0' + (irq % 10));
    
    // Acknowledge the interrupt to the PIC(s)
    if (irq >= 8) {
        // If this was an IRQ from the slave PIC, send EOI to both PICs
        outb(0xA0, 0x20);  // Send EOI to slave
        outb(0x20, 0x20);  // Send EOI to master
    } else {
        // If this was an IRQ from the master PIC, just send EOI to master
        outb(0x20, 0x20);
    }
}

// Set an IDT gate
static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low = (base & 0xFFFF);
    idt[num].selector = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
    idt[num].base_high = (base >> 16) & 0xFFFF;
}


// Initialize the Programmable Interrupt Controller (PIC)
static void init_pic() {
    // Save masks
    uint8_t a1 = inb(0x21);
    uint8_t a2 = inb(0xA1);
    
    // Start initialization sequence (cascade mode)
    outb(0x20, 0x11);
    io_wait();
    outb(0xA0, 0x11);
    io_wait();

    // Remap IRQ0-IRQ7 to 0x20-0x27 and IRQ8-IRQ15 to 0x28-0x2F
    outb(0x21, 0x20);  // Master PIC vector offset
    io_wait();
    outb(0xA1, 0x28);  // Slave PIC vector offset
    io_wait();

    // Tell Master PIC about the slave at IRQ2 (0000 0100)
    outb(0x21, 0x04);
    io_wait();
    // Tell Slave PIC its cascade identity (0000 0010)
    outb(0xA1, 0x02);
    io_wait();

    // Set 8086/88 mode
    outb(0x21, 0x01);
    io_wait();
    outb(0xA1, 0x01);
    io_wait();

    // Restore saved masks
    outb(0x21, a1);
    outb(0xA1, a2);
    
    // Mask all interrupts except cascade (IRQ2) on master
    outb(0x21, 0xFB);  // 1111 1011 - Only IRQ2 (slave) is unmasked
    // Mask all interrupts on slave
    outb(0xA1, 0xFF);  // 1111 1111 - All masked
}

// Initialize the IDT
void idt_init() {
    // Set up the IDT pointer
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (uint32_t)&idt;

    // Clear out the entire IDT
    uint8_t *idt_bytes = (uint8_t *)&idt;
    for (size_t i = 0; i < sizeof(struct idt_entry) * 256; i++) {
        idt_bytes[i] = 0;
    }

    // Set up the exception handlers (0-31)
    for (int i = 0; i < 32; i++) {
        idt_set_gate(i, (uint32_t)default_exception_handler, 0x08, 0x8E);
    }

    // Set up IRQ handlers (32-47)
    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);  // IRQ0: Timer
    // Add more IRQ handlers as needed
    for (int i = 33; i < 48; i++) {
        idt_set_gate(i, (uint32_t)default_irq_handler, 0x08, 0x8E);
    }

    // Initialize the PIC
    init_pic();

    // Load the IDT
    idt_load((uint32_t)&idtp);
    
    // Enable interrupts
    asm volatile ("sti");
}
