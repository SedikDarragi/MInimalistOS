#include "vga.h"

static uint16_t* vga_buffer = (uint16_t*)0xB8000;
static int cursor_x = 0;
static int cursor_y = 0;
static uint8_t current_color = 0x07; // Light grey on black

static uint8_t vga_entry_color(vga_color_t fg, vga_color_t bg) {
    return fg | bg << 4;
}

static uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t)uc | (uint16_t)color << 8;
}

void vga_init(void) {
    cursor_x = 0;
    cursor_y = 0;
    current_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

void vga_clear(void) {
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            const int index = y * VGA_WIDTH + x;
            vga_buffer[index] = vga_entry(' ', current_color);
        }
    }
    cursor_x = 0;
    cursor_y = 0;
    vga_set_cursor(0, 0);
}

void vga_set_color(vga_color_t fg, vga_color_t bg) {
    current_color = vga_entry_color(fg, bg);
}

void vga_putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\t') {
        cursor_x = (cursor_x + 8) & ~(8 - 1);
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            const int index = cursor_y * VGA_WIDTH + cursor_x;
            vga_buffer[index] = vga_entry(' ', current_color);
        }
    } else {
        const int index = cursor_y * VGA_WIDTH + cursor_x;
        vga_buffer[index] = vga_entry(c, current_color);
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
    
    vga_set_cursor(cursor_x, cursor_y);
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
    for (int y = 1; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[(y - 1) * VGA_WIDTH + x] = vga_buffer[y * VGA_WIDTH + x];
        }
    }
    
    // Clear last line
    for (int x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', current_color);
    }
}

void vga_set_cursor(int x, int y) {
    uint16_t pos = y * VGA_WIDTH + x;
    
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}
