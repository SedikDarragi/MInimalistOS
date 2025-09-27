#include <stdint.h>

// VGA text mode buffer
volatile uint16_t* vga_buffer = (volatile uint16_t*)0xB8000;

// Simple VGA text output
void vga_puts(const char* str) {
    static uint16_t cursor_x = 0;
    static uint16_t cursor_y = 0;
    
    for (int i = 0; str[i]; i++) {
        if (str[i] == '\n') {
            cursor_x = 0;
            cursor_y++;
            continue;
        }
        
        // Write character with white on black
        vga_buffer[cursor_y * 80 + cursor_x] = (0x0F << 8) | str[i];
        
        cursor_x++;
        if (cursor_x >= 80) {
            cursor_x = 0;
            cursor_y++;
        }
        
        // Simple screen scrolling
        if (cursor_y >= 25) {
            // Move all lines up
            for (int y = 0; y < 24; y++) {
                for (int x = 0; x < 80; x++) {
                    vga_buffer[y * 80 + x] = vga_buffer[(y + 1) * 80 + x];
                }
            }
            // Clear the last line
            for (int x = 0; x < 80; x++) {
                vga_buffer[24 * 80 + x] = (0x0F << 8) | ' ';
            }
            cursor_y = 24;
        }
    }
}

// Clear the screen
void vga_clear(void) {
    for (int i = 0; i < 80 * 25; i++) {
        vga_buffer[i] = (0x0F << 8) | ' ';
    }
}

// Kernel entry point
void _start(void) {
    // Clear the screen
    vga_clear();
    
    // Print welcome message
    vga_puts("MinimalOS Kernel Loaded!\n");
    vga_puts("Successfully booted into protected mode.\n");
    vga_puts("System is running...\n");
    
    // Halt the CPU
    while (1) {
        asm volatile ("hlt");
    }
}
