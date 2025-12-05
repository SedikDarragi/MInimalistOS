#include "../include/vga.h"

// Simple test process that counts and displays
void test_process_1(void) {
    int count = 0;
    volatile uint16_t* vga = (volatile uint16_t*)0xB8000;
    
    while (1) {
        // Display count at different positions
        vga[80*12 + 10] = 0x0A00 + ((count % 10) + '0');
        count++;
        
        // Simple delay loop
        for (volatile int i = 0; i < 100000; i++);
    }
}

void test_process_2(void) {
    int count = 0;
    volatile uint16_t* vga = (volatile uint16_t*)0xB8000;
    
    while (1) {
        // Display count at different positions
        vga[80*13 + 10] = 0x0C00 + ((count % 10) + '0');
        count++;
        
        // Simple delay loop
        for (volatile int i = 0; i < 150000; i++);
    }
}
