#include "../drivers/vga.h"
#include "io.h"
#include "log.h"

// ISR (Interrupt Service Routine) stubs
void isr0() {
    // Division by zero exception
    log_critical("Division by zero exception!");
    panic("Division by zero");
}

void isr1() {
    // Debug exception
    log_debug("Debug exception occurred");
}

void isr2() {
    // Non-maskable interrupt
    log_critical("Non-maskable interrupt received!");
    log_info("Hardware failure detected");
}

void isr3() {
    // Breakpoint exception
    log_debug("Breakpoint exception - debugger breakpoint hit");
}

void isr4() {
    // Overflow exception
    log_error("Overflow exception detected");
}

void isr5() {
    // Bound range exceeded exception
    log_error("Bound range exceeded exception");
}

void isr6() {
    // Invalid opcode exception
    log_error("Invalid opcode exception - corrupted instruction");
}

void isr7() {
    // Device not available exception
    log_error("Device not available - FPU or coprocessor error");
}

void isr8() {
    // Double fault exception
    log_critical("Double fault exception - critical system error!");
    panic("Double fault");
}

void isr9() {
    // Coprocessor segment overrun
    log_error("Coprocessor segment overrun");
}

void isr10() {
    // Invalid TSS exception
    log_error("Invalid TSS exception - task state segment error");
}

void isr11() {
    // Segment not present exception
    log_error("Segment not present exception");
}

void isr12() {
    // Stack segment fault
    log_error("Stack segment fault - stack corruption");
}

void isr13() {
    // General protection fault
    log_error("General protection fault - memory access violation");
}

void isr14() {
    // Page fault
    log_error("Page fault - invalid memory access");
}

void isr15() {
    // Reserved exception
    log_error("Reserved exception");
}

void isr16() {
    // x87 floating-point exception
    log_error("x87 floating-point exception");
}

void isr17() {
    // Alignment check exception
    log_error("Alignment check exception");
}

void isr18() {
    // Machine check exception
    log_critical("Machine check exception - hardware error!");
    panic("Machine check");
}

void isr19() {
    // SIMD floating-point exception
    log_error("SIMD floating-point exception");
}

void isr20() {
    // Virtualization exception
    log_error("Virtualization exception");
}

void isr21() {
    // Reserved exception
    log_error("Reserved exception");
}

void isr22() {
    // Reserved exception
    log_error("Reserved exception");
}

void isr23() {
    // Reserved exception
    log_error("Reserved exception");
}

void isr24() {
    // Reserved exception
    log_error("Reserved exception");
}

void isr25() {
    // Reserved exception
    log_error("Reserved exception");
}

void isr26() {
    // Reserved exception
    log_error("Reserved exception");
}

void isr27() {
    // Reserved exception
    log_error("Reserved exception");
}

void isr28() {
    // Reserved exception
    log_error("Reserved exception");
}

void isr29() {
    // Reserved exception
    log_error("Reserved exception");
}

void isr30() {
    // Reserved exception
    log_error("Reserved exception");
}

void isr31() {
    // Reserved exception
    log_error("Reserved exception");
}

// IRQ (Interrupt Request) handlers
void irq0() {
    // Timer
    static uint32_t tick_count = 0;
    tick_count++;
    if (tick_count % 100 == 0) {
        log_debug("Timer tick");
    }
    // Send EOI to PIC
    outb(0x20, 0x20);
}

void irq1() {
    // Keyboard
    uint8_t scancode = inb(0x60);
    (void)scancode; // Suppress unused variable warning
    log_debug("Keyboard scancode received");
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
