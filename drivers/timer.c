#include "timer.h"

static volatile uint32_t tick_count = 0;

void timer_init(void) {
    // Set up PIT (Programmable Interval Timer)
    uint32_t divisor = 1193180 / 100; // 100 Hz
    
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, divisor >> 8);
}

void timer_wait(uint32_t ticks) {
    uint32_t start = tick_count;
    while (tick_count - start < ticks) {
        __asm__ volatile ("hlt");
    }
}

uint32_t timer_get_ticks(void) {
    return tick_count;
}

// Timer interrupt handler (would be called by interrupt system)
void timer_handler(void) {
    tick_count++;
}
