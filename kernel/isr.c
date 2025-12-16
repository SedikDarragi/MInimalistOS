#include "../drivers/vga.h"
#include "io.h"

// ISR (Interrupt Service Routine) stubs
void isr0() {
    // Division by zero exception
    vga_print("Division by zero exception!\n");
    while(1) {}
}

void isr1() {
    // Debug exception
    vga_print("Debug exception!\n");
    while(1) {}
}

void isr2() {
    // Non-maskable interrupt
    vga_print("Non-maskable interrupt!\n");
    while(1) {}
}

void isr3() {
    // Breakpoint exception
    vga_print("Breakpoint exception!\n");
    while(1) {}
}

void isr4() {
    // Overflow exception
    vga_print("Overflow exception!\n");
    while(1) {}
}

void isr5() {
    // Bound range exceeded exception
    vga_print("Bound range exceeded exception!\n");
    while(1) {}
}

void isr6() {
    // Invalid opcode exception
    vga_print("Invalid opcode exception!\n");
    while(1) {}
}

void isr7() {
    // Device not available exception
    vga_print("Device not available exception!\n");
    while(1) {}
}

void isr8() {
    // Double fault exception
    vga_print("Double fault exception!\n");
    while(1) {}
}

void isr9() {
    // Coprocessor segment overrun
    vga_print("Coprocessor segment overrun!\n");
    while(1) {}
}

void isr10() {
    // Invalid TSS exception
    vga_print("Invalid TSS exception!\n");
    while(1) {}
}

void isr11() {
    // Segment not present exception
    vga_print("Segment not present exception!\n");
    while(1) {}
}

void isr12() {
    // Stack-segment fault exception
    vga_print("Stack-segment fault exception!\n");
    while(1) {}
}

void isr13() {
    // General protection fault exception
    vga_print("General protection fault exception!\n");
    while(1) {}
}

void isr14() {
    // Page fault exception
    vga_print("Page fault exception!\n");
    while(1) {}
}

void isr15() {
    // Reserved exception
    vga_print("Reserved exception!\n");
    while(1) {}
}

void isr16() {
    // x87 floating-point exception
    vga_print("x87 floating-point exception!\n");
    while(1) {}
}

void isr17() {
    // Alignment check exception
    vga_print("Alignment check exception!\n");
    while(1) {}
}

void isr18() {
    // Machine check exception
    vga_print("Machine check exception!\n");
    while(1) {}
}

void isr19() {
    // SIMD floating-point exception
    vga_print("SIMD floating-point exception!\n");
    while(1) {}
}

void isr20() {
    // Virtualization exception
    vga_print("Virtualization exception!\n");
    while(1) {}
}

void isr21() {
    // Reserved exception
    vga_print("Reserved exception!\n");
    while(1) {}
}

void isr22() {
    // Reserved exception
    vga_print("Reserved exception!\n");
    while(1) {}
}

void isr23() {
    // Reserved exception
    vga_print("Reserved exception!\n");
    while(1) {}
}

void isr24() {
    // Reserved exception
    vga_print("Reserved exception!\n");
    while(1) {}
}

void isr25() {
    // Reserved exception
    vga_print("Reserved exception!\n");
    while(1) {}
}

void isr26() {
    // Reserved exception
    vga_print("Reserved exception!\n");
    while(1) {}
}

void isr27() {
    // Reserved exception
    vga_print("Reserved exception!\n");
    while(1) {}
}

void isr28() {
    // Reserved exception
    vga_print("Reserved exception!\n");
    while(1) {}
}

void isr29() {
    // Reserved exception
    vga_print("Reserved exception!\n");
    while(1) {}
}

void isr30() {
    // Reserved exception
    vga_print("Reserved exception!\n");
    while(1) {}
}

void isr31() {
    // Reserved exception
    vga_print("Reserved exception!\n");
    while(1) {}
}

// IRQ (Interrupt Request) handlers
void irq0() {
    // Timer (PIT)
    // Send EOI to PIC
    outb(0x20, 0x20);
}

void irq1() {
    // Keyboard
    // Send EOI to PIC
    outb(0x20, 0x20);
}

void irq2() {
    // Cascade (used internally by PIC)
    outb(0x20, 0x20);
}

void irq3() {
    // COM2/COM4
    outb(0x20, 0x20);
}

void irq4() {
    // COM1/COM3
    outb(0x20, 0x20);
}

void irq5() {
    // LPT2
    outb(0x20, 0x20);
}

void irq6() {
    // Floppy disk
    outb(0x20, 0x20);
}

void irq7() {
    // LPT1
    outb(0x20, 0x20);
}

void irq8() {
    // CMOS RTC
    outb(0x20, 0x20);
    outb(0xA0, 0x20);
}

void irq9() {
    // Free for peripherals
    outb(0x20, 0x20);
    outb(0xA0, 0x20);
}

void irq10() {
    // Free for peripherals
    outb(0x20, 0x20);
    outb(0xA0, 0x20);
}

void irq11() {
    // Free for peripherals
    outb(0x20, 0x20);
    outb(0xA0, 0x20);
}

void irq12() {
    // Mouse
    outb(0x20, 0x20);
    outb(0xA0, 0x20);
}

void irq13() {
    // FPU
    outb(0x20, 0x20);
    outb(0xA0, 0x20);
}

void irq14() {
    // Primary ATA
    outb(0x20, 0x20);
    outb(0xA0, 0x20);
}

void irq15() {
    // Secondary ATA
    outb(0x20, 0x20);
    outb(0xA0, 0x20);
}
