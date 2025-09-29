#include <stdint.h>
#include <stddef.h>

// VGA text mode memory address
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((volatile uint16_t*)0xB8000)

// Text mode color constants
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_YELLOW = 14,
    VGA_COLOR_WHITE = 15,
};

// Create a color byte from foreground and background colors
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | (bg << 4);
}

// Create a VGA entry from a character and color
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t)uc | ((uint16_t)color << 8);
}

// Clear the screen with a specific color
void vga_clear(uint8_t color) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            VGA_MEMORY[index] = vga_entry(' ', color);
        }
    }
}

// Write a string to the screen at a specific position
void vga_puts(const char* str, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    for (size_t i = 0; str[i] != '\0'; i++) {
        VGA_MEMORY[index + i] = vga_entry(str[i], color);
    }
}

// Kernel entry point (called from assembly)
void kmain(void) {
    // Set up colors (white on blue)
    const uint8_t color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    
    // Clear the screen
    vga_clear(color);
    
    // Write some messages
    vga_puts("MinimalOS Kernel", color, 32, 10);
    vga_puts("Successfully booted into protected mode!", color, 20, 12);
    vga_puts("System is running...", color, 30, 14);
    
    // Halt the CPU
    while (1) {
        __asm__ volatile ("hlt");
    }
}
