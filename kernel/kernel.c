#include "kernel.h"
#include "../drivers/vga.h"
#include "../drivers/keyboard.h"
#include "../drivers/timer.h"
#include "../fs/filesystem.h"
#include "../net/network.h"
#include "../ui/ui.h"
#include "shell.h"

// Simple serial output function for debugging
void serial_putc(char c) {
    // Wait until the serial port is ready
    while ((inb(0x3F8 + 5) & 0x20) == 0);
    outb(0x3F8, c);
}

void serial_print(const char* str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        serial_putc(str[i]);
    }
}

#include <stdint.h>

// Simple VGA output function
void vga_write(const char* str) {
    volatile uint16_t* vga_memory = (volatile uint16_t*)0xB8000;
    static int x = 0, y = 0;
    
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\n') {
            x = 0;
            y++;
            continue;
        }
        
        // White on blue for better visibility
        vga_memory[y * 80 + x] = (0x1F << 8) | str[i];
        x++;
        
        if (x >= 80) {
            x = 0;
            y++;
        }
        
        if (y >= 25) {
            // Simple scroll
            for (int j = 0; j < 24 * 80; j++) {
                vga_memory[j] = vga_memory[j + 80];
            }
            // Clear the last line
            for (int j = 24 * 80; j < 25 * 80; j++) {
                vga_memory[j] = 0x1F00;
            }
            y = 24;
        }
    }
}

void __attribute__((noreturn)) kmain(void) {
    // Clear screen with blue background
    volatile uint16_t* vga = (volatile uint16_t*)0xB8000;
    for (int i = 0; i < 80 * 25; i++) {
        vga[i] = 0x1F00;  // Blue background, black space
    }
    
    // Display a simple message
    vga_write("\n\n  MinimalOS v1.0 - Kernel is running!\n");
    vga_write("  ================================\n\n");
    vga_write("  Kernel initialized successfully!\n\n");
    vga_write("  System Halted.\n");
    
    // Halt the CPU
    while (1) {
        asm volatile ("hlt");
    }
    
    // This should never be reached due to the infinite loop above
    __builtin_unreachable();
}
