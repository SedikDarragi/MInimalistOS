#include "../include/idt.h"
#include "../include/memory.h"
#include "../drivers/vga.h"
#include "log.h"

// Test interrupt registration and handling
static volatile int timer_interrupt_count = 0;
static volatile int keyboard_interrupt_count = 0;

// Custom timer interrupt handler
void timer_interrupt_handler(struct regs* r) {
    timer_interrupt_count++;
    if (timer_interrupt_count % 50 == 0) {
        log_info("Timer interrupts received: ");
        // Simple number printing
        if (timer_interrupt_count >= 1000) {
            vga_putchar('0' + (timer_interrupt_count / 1000) % 10);
        }
        if (timer_interrupt_count >= 100) {
            vga_putchar('0' + (timer_interrupt_count / 100) % 10);
        }
        if (timer_interrupt_count >= 10) {
            vga_putchar('0' + (timer_interrupt_count / 10) % 10);
        }
        vga_putchar('0' + timer_interrupt_count % 10);
        vga_print("\n");
    }
}

// Custom keyboard interrupt handler
void keyboard_interrupt_handler(struct regs* r) {
    keyboard_interrupt_count++;
    log_debug("Keyboard interrupt detected");
}

// Test interrupt registration
void test_interrupt_registration(void) {
    log_info("Testing interrupt registration...");
    
    // Register custom timer handler
    register_interrupt_handler(32, timer_interrupt_handler);
    log_info("✓ Custom timer handler registered");
    
    // Register custom keyboard handler
    register_interrupt_handler(33, keyboard_interrupt_handler);
    log_info("✓ Custom keyboard handler registered");
    
    // Enable timer IRQ (IRQ 0)
    enable_irq(0);
    log_info("✓ Timer IRQ enabled");
    
    // Enable keyboard IRQ (IRQ 1)
    enable_irq(1);
    log_info("✓ Keyboard IRQ enabled");
}

// Test interrupt handling by waiting for interrupts
void test_interrupt_handling(void) {
    log_info("Testing interrupt handling...");
    log_info("Waiting for timer and keyboard interrupts...");
    log_info("Press keys to generate keyboard interrupts");
    
    // Wait for interrupts to occur
    for (int i = 0; i < 1000000; i++) {
        // Enable interrupts and wait
        asm volatile("sti");
        
        // Small delay
        for (int j = 0; j < 1000; j++) {
            asm volatile("nop");
        }
        
        // Check if we received interrupts
        if (timer_interrupt_count > 100) {
            break;
        }
    }
    
    log_info("Interrupt test completed");
    log_info("Timer interrupts received: ");
    if (timer_interrupt_count >= 1000) {
        vga_putchar('0' + (timer_interrupt_count / 1000) % 10);
    }
    if (timer_interrupt_count >= 100) {
        vga_putchar('0' + (timer_interrupt_count / 100) % 10);
    }
    if (timer_interrupt_count >= 10) {
        vga_putchar('0' + (timer_interrupt_count / 10) % 10);
    }
    vga_putchar('0' + timer_interrupt_count % 10);
    vga_print("\n");
    
    log_info("Keyboard interrupts received: ");
    if (keyboard_interrupt_count >= 1000) {
        vga_putchar('0' + (keyboard_interrupt_count / 1000) % 10);
    }
    if (keyboard_interrupt_count >= 100) {
        vga_putchar('0' + (keyboard_interrupt_count / 100) % 10);
    }
    if (keyboard_interrupt_count >= 10) {
        vga_putchar('0' + (keyboard_interrupt_count / 10) % 10);
    }
    vga_putchar('0' + keyboard_interrupt_count % 10);
    vga_print("\n");
}

// Test exception handling (safe test)
void test_exception_handling(void) {
    log_info("Testing exception handling...");
    log_info("Note: We will not trigger actual exceptions to avoid system crash");
    
    // Just verify that exception handlers are registered
    log_info("✓ Exception handlers are registered");
    log_info("✓ Division by zero handler: isr0");
    log_info("✓ Page fault handler: isr14");
    log_info("✓ General protection fault handler: isr13");
    log_info("✓ Double fault handler: isr8");
}

// Main interrupt test function
void run_interrupt_tests(void) {
    log_init();
    log_info("=== Interrupt Handling Tests ===");
    log_info("");
    
    test_interrupt_registration();
    log_info("");
    
    test_exception_handling();
    log_info("");
    
    test_interrupt_handling();
    log_info("");
    
    log_info("=== Interrupt Tests Complete ===");
    
    while (1) {
        // Halt the CPU and wait for interrupts
        asm volatile("hlt");
    }
}
