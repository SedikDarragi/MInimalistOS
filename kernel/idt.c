#include <stdint.h>

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

// Default exception handler
static void default_exception_handler() {
    // Just halt the CPU for now
    asm volatile ("hlt");
}

// Set an IDT gate
static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low = (base & 0xFFFF);
    idt[num].selector = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
    idt[num].base_high = (base >> 16) & 0xFFFF;
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

    // Load the IDT
    idt_load((uint32_t)&idtp);
}
