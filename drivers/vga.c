#include "vga.h"
#include <stdint.h>
#include <stddef.h>

// VGA memory address
#define VGA_MEMORY 0xB8000

// Current cursor position
static int cursor_x = 0;
static int cursor_y = 0;
static vga_color_t current_color = VGA_COLOR_LIGHT_GREY;

// Helper function to write to VGA memory
static inline void vga_putchar_at(char c, uint8_t color, int x, int y) {
    uint16_t* vga = (uint16_t*)VGA_MEMORY;
    vga[y * VGA_WIDTH + x] = (uint16_t)c | (uint16_t)color << 8;
}

void vga_init(void) {
    vga_clear();
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

void vga_clear(void) {
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_putchar_at(' ', current_color, x, y);
        }
    }
    cursor_x = 0;
    cursor_y = 0;
}

void vga_set_color(vga_color_t fg, vga_color_t bg) {
    (void)bg; // Suppress unused parameter warning
    current_color = fg;
}

void vga_putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\t') {
        cursor_x = (cursor_x + 8) & ~7;
    } else if (c >= ' ') {
        vga_putchar_at(c, current_color, cursor_x, cursor_y);
        cursor_x++;
    }
    
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }
    
    if (cursor_y >= VGA_HEIGHT) {
        vga_scroll();
        cursor_y = VGA_HEIGHT - 1;
    }
}

void vga_print(const char* str) {
    while (*str) {
        vga_putchar(*str++);
    }
}

void vga_print_at(const char* str, int x, int y) {
    int old_x = cursor_x;
    int old_y = cursor_y;
    cursor_x = x;
    cursor_y = y;
    vga_print(str);
    cursor_x = old_x;
    cursor_y = old_y;
}

void vga_scroll(void) {
    // Move everything up one line
    uint16_t* vga = (uint16_t*)VGA_MEMORY;
    for (int y = 0; y < VGA_HEIGHT - 1; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga[y * VGA_WIDTH + x] = vga[(y + 1) * VGA_WIDTH + x];
        }
    }
    
    // Clear the last line
    for (int x = 0; x < VGA_WIDTH; x++) {
        vga_putchar_at(' ', current_color, x, VGA_HEIGHT - 1);
    }
}

void vga_set_cursor(int x, int y) {
    if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT) {
        cursor_x = x;
        cursor_y = y;
    }
}
