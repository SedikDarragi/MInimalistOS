#include "timer.h"
#include "../kernel/kernel.h"
#include "../include/idt.h"

#define PIT_CMD_PORT 0x43
#define PIT_CHANNEL0 0x40
#define PIT_FREQUENCY 100  // 100Hz timer

volatile uint32_t timer_ticks = 0;

// Timer interrupt handler
void timer_interrupt_handler(struct regs* r) {
    (void)r; // Suppress unused parameter warning
    timer_ticks++;
    
    // Display timer tick count every 100 ticks (1 second)
    if (timer_ticks % 100 == 0) {
        // Simple VGA output to show timer is working
        volatile uint16_t* vga = (volatile uint16_t*)0xB8000;
        vga[80*24 + 70] = 0x1F00 + ((timer_ticks / 100) % 10) + '0';
    }
    
    // Add process scheduling here later
    // if (timer_ticks % 10 == 0) {
    //     schedule();
    // }
}

void timer_init(void) {
    // Register the timer handler
    register_interrupt_handler(32, timer_interrupt_handler);  // IRQ0 = 32
    
    // Set up PIT (Programmable Interval Timer)
    uint32_t divisor = 1193180 / PIT_FREQUENCY; // 100 Hz
    
    outb(PIT_CMD_PORT, 0x36);
    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, divisor >> 8);
    
    // Enable IRQ0 (timer)
    enable_irq(0);
}

void timer_wait(uint32_t ticks) {
    uint32_t start = timer_ticks;
    while (timer_ticks - start < ticks) {
        __asm__ volatile ("hlt");
    }
}

uint32_t timer_get_ticks(void) {
    return timer_ticks;
}
