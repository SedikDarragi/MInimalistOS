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

// VGA text buffer
#define VGA_BUFFER ((volatile uint16_t*)0xB8000)

// Simple hex to character conversion
static char hex_chars[] = "0123456789ABCDEF";

// Default IRQ handler
void default_irq_handler(uint32_t irq) {
    // Clear the first 10 lines of the screen with a different background
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 80; x++) {
            VGA_BUFFER[y * 80 + x] = 0x1F00;  // Blue background, black space
        }
    }
    
    // Display a header
    const char *header = "=== MINIMAL OS DEBUG ===";
    for (int i = 0; header[i]; i++) {
        VGA_BUFFER[i] = 0x1F00 | header[i];
    }
    
    // Display IRQ information on line 2
    char irq_msg[32];
    irq_msg[0] = 'I';
    irq_msg[1] = 'R';
    irq_msg[2] = 'Q';
    irq_msg[3] = '0' + (irq / 10);
    irq_msg[4] = '0' + (irq % 10);
    irq_msg[5] = '\0';
    
    for (int i = 0; irq_msg[i]; i++) {
        VGA_BUFFER[1 * 80 + i] = 0x1F00 | irq_msg[i];
    }
    
    // Read ISR value
    outb(0x20, 0x0B);  // Read ISR on master
    uint8_t isr = inb(0x20);
    
    // Display ISR value on line 3
    const char *isr_str = "ISR=0x";
    for (int i = 0; isr_str[i]; i++) {
        VGA_BUFFER[2 * 80 + i] = 0x1F00 | isr_str[i];
    }
    
    // Display ISR in hex on line 3
    VGA_BUFFER[2 * 80 + 6] = 0x1F00 | hex_chars[(isr >> 4) & 0xF];
    VGA_BUFFER[2 * 80 + 7] = 0x1F00 | hex_chars[isr & 0xF];
    
    // Display a message on line 5
    const char *msg = "Interrupt received!";
    for (int i = 0; msg[i]; i++) {
        VGA_BUFFER[4 * 80 + i] = 0x1F00 | msg[i];
    }
    
    // Acknowledge the interrupt to the PIC(s)
    if (irq >= 8) {
        outb(0xA0, 0x20);  // Send EOI to slave
        outb(0x20, 0x20);  // Send EOI to master
    } else {
        outb(0x20, 0x20);  // Send EOI to master only
    }
    
    // Small delay to make the output readable
    for (volatile int i = 0; i < 1000000; i++);
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
