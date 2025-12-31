#include "mouse.h"
#include "../kernel/io.h"
#include "../include/idt.h"
#include "../kernel/log.h"
#include "../drivers/vga.h"

// PS/2 controller ports (same as keyboard)
#define PS2_DATA_PORT   0x60
#define PS2_STATUS_PORT 0x64
#define PS2_CMD_PORT    0x64

// Mouse uses IRQ12 -> interrupt vector 32 + 12 = 44
#define MOUSE_IRQ_VECTOR 44
#define MOUSE_IRQ_LINE   12

// Simple 3-byte PS/2 mouse packet state
static uint8_t packet[3];
static int packet_index = 0;

static int mouse_x = 0;
static int mouse_y = 0;
static uint8_t mouse_buttons = 0;

// Wait helper: type 0 = wait to write, type 1 = wait to read
void mouse_wait(uint8_t type) {
    int timeout = 100000;
    if (type == 0) {
        // Wait for input buffer to be clear (bit 1 == 0)
        while (timeout-- && (inb(PS2_STATUS_PORT) & 0x02));
    } else {
        // Wait for output buffer to be full (bit 0 == 1)
        while (timeout-- && !(inb(PS2_STATUS_PORT) & 0x01));
    }
}

void mouse_write(uint8_t data) {
    // Tell controller next byte is for mouse
    mouse_wait(0);
    outb(PS2_CMD_PORT, 0xD4);
    mouse_wait(0);
    outb(PS2_DATA_PORT, data);
}

uint8_t mouse_read(void) {
    mouse_wait(1);
    return inb(PS2_DATA_PORT);
}

// Very simple on-screen cursor: single character drawn via vga_print_at
static void draw_cursor(void) {
    // Clamp to screen
    if (mouse_x < 0) mouse_x = 0;
    if (mouse_y < 0) mouse_y = 0;
    if (mouse_x >= VGA_WIDTH)  mouse_x = VGA_WIDTH - 1;
    if (mouse_y >= VGA_HEIGHT) mouse_y = VGA_HEIGHT - 1;

    char s[2] = {'X', '\0'};
    vga_print_at(s, mouse_x, mouse_y);
}

static void clear_cursor(void) {
    if (mouse_x < 0 || mouse_y < 0 || mouse_x >= VGA_WIDTH || mouse_y >= VGA_HEIGHT)
        return;
    char s[2] = {' ', '\0'};
    vga_print_at(s, mouse_x, mouse_y);
}

// IRQ12 handler (called via IDT/irq_common_stub)
static void mouse_irq_handler(struct regs* r) {
    (void)r;

    uint8_t status = inb(PS2_STATUS_PORT);
    if (!(status & 0x20)) {
        // Not from mouse
        return;
    }

    uint8_t data = inb(PS2_DATA_PORT);

    // First byte must always have bit 3 set for a valid packet
    if (packet_index == 0 && !(data & 0x08)) {
        return;
    }

    packet[packet_index++] = data;

    if (packet_index < 3)
        return;

    // We have a full 3-byte packet
    packet_index = 0;

    int dx = (int8_t)packet[1];
    int dy = (int8_t)packet[2];

    clear_cursor();

    mouse_x += dx;
    mouse_y -= dy; // PS/2 Y is opposite screen Y

    mouse_buttons = packet[0] & 0x07; // left/mid/right

    draw_cursor();
}

void mouse_init(void) {
    // Enable auxiliary device
    mouse_wait(0);
    outb(PS2_CMD_PORT, 0xA8);

    // Enable IRQ12 in controller
    mouse_wait(0);
    outb(PS2_CMD_PORT, 0x20); // read Controller Command Byte
    mouse_wait(1);
    uint8_t status = inb(PS2_DATA_PORT);
    status |= 0x02; // enable IRQ12 (mouse)
    mouse_wait(0);
    outb(PS2_CMD_PORT, 0x60);
    mouse_wait(0);
    outb(PS2_DATA_PORT, status);

    // Reset internal state
    packet_index = 0;
    mouse_x = 0;
    mouse_y = 0;
    mouse_buttons = 0;

    // Set defaults and enable streaming
    mouse_write(0xF6); // Set defaults
    mouse_read();      // ACK
    mouse_write(0xF4); // Enable data reporting
    mouse_read();      // ACK

    // Register handler for vector 32+12
    register_interrupt_handler(MOUSE_IRQ_VECTOR, mouse_irq_handler);

    // Unmask IRQ12 on PIC
    enable_irq(MOUSE_IRQ_LINE);

    vga_print("Mouse: initialized\n");
    log_info("Mouse driver initialized");
}

int mouse_get_x(void) { return mouse_x; }
int mouse_get_y(void) { return mouse_y; }
int mouse_get_buttons(void) { return mouse_buttons; }
