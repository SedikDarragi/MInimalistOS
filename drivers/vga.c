#include "vga.h"
#include <stdint.h>
#include <stddef.h>

#include "../kernel/io.h"

#define VGA_MEMORY 0xB8000

static int cursor_x = 0;
static int cursor_y = 0;
static uint8_t current_color = 0x07;

// Expose cursor position for external use
int vga_get_cursor_y(void) {
    return cursor_y;
}

static void vga_update_cursor(void) {
    uint16_t pos = (uint16_t)(cursor_y * VGA_WIDTH + cursor_x);
    
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
}

static inline void vga_putchar_at(char c, uint8_t color, int x, int y) {
    volatile uint16_t* vga = (volatile uint16_t*)VGA_MEMORY;
    vga[y * VGA_WIDTH + x] = (uint16_t)c | (uint16_t)color << 8;
}

void vga_enable_cursor(uint8_t cursor_start, uint8_t cursor_end) {
    outb(0x3D4, 0x0A);
    // Bit 5 of 0x0A is the Cursor Disable bit. We force it to 0 to enable.
    outb(0x3D5, (0x00 & 0xC0) | (cursor_start & 0x1F));
    
    outb(0x3D4, 0x0B);
    outb(0x3D5, (0x00 & 0xE0) | (cursor_end & 0x1F));
}

void vga_init(void) {
    // 0 to 15 creates a full block cursor; 14 to 15 is a thin underline.
    vga_enable_cursor(0, 15);
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_clear();
    vga_update_cursor();
}

void vga_clear(void) {
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_putchar_at(' ', current_color, x, y);
        }
    }
    cursor_x = 0;
    cursor_y = 0;
    vga_update_cursor();
}

void vga_set_color(vga_color_t fg, vga_color_t bg) {
    current_color = (uint8_t)((bg << 4) | (fg & 0x0F));
}

void vga_putchar(char c) {
    // Ensure cursor is within bounds before processing
    if (cursor_x >= VGA_WIDTH) cursor_x = 0;
    if (cursor_y >= VGA_HEIGHT) cursor_y = VGA_HEIGHT - 1;

    if (c == '\n' || c == '\r') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
        } else if (cursor_y > 0) {
            cursor_y--;
            cursor_x = VGA_WIDTH - 1;
        }
        vga_putchar_at(' ', current_color, cursor_x, cursor_y);
    } else if (c == '\t') {
        cursor_x = (cursor_x + 8) & ~7;
    } else if (c >= ' ') {
        // If we're at the end of a line, automatically wrap
        if (cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
        }
        vga_putchar_at(c, current_color, cursor_x, cursor_y);
        cursor_x++;
    }
    
    while (cursor_y >= VGA_HEIGHT) {
        vga_scroll();
        cursor_y--;
    }

    vga_update_cursor();
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
    vga_update_cursor();
}

void vga_scroll(void) {
    volatile uint16_t* vga = (volatile uint16_t*)VGA_MEMORY;

    // Shift lines up
    for (int y = 0; y < VGA_HEIGHT - 1; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga[y * VGA_WIDTH + x] = vga[(y + 1) * VGA_WIDTH + x];
        }
    }
    
    // Clear the bottom line
    for (int x = 0; x < VGA_WIDTH; x++) {
        vga_putchar_at(' ', current_color, x, VGA_HEIGHT - 1);
    }
}

void vga_set_cursor(int x, int y) {
    if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT) {
        cursor_x = x;
        cursor_y = y;
        vga_update_cursor();
    }
}
