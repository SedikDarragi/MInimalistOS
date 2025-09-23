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

void __attribute__((noreturn)) kmain(void) {
    // Initialize serial port for debugging
    outb(0x3F8 + 1, 0x00);    // Disable all interrupts
    outb(0x3F8 + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(0x3F8 + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(0x3F8 + 1, 0x00);    //                  (hi byte)
    outb(0x3F8 + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(0x3F8 + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(0x3F8 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
    
    serial_print("SERIAL: Starting kernel...\n");
    
    // Initialize VGA display immediately
    serial_print("SERIAL: Initializing VGA...\n");
    vga_init();
    vga_clear();
    
    // Print immediate debug message
    vga_print("KERNEL STARTED!\n");
    vga_print("MinimalOS v1.0\n");
    vga_print("Initializing system...\n");
    serial_print("SERIAL: Kernel initialized, starting drivers...\n");
    
    // Initialize drivers
    serial_print("SERIAL: Initializing keyboard...\n");
    keyboard_init();
    serial_print("SERIAL: Initializing timer...\n");
    timer_init();
    
    // Initialize filesystem
    serial_print("SERIAL: Initializing filesystem...\n");
    fs_init();
    vga_print("Filesystem initialized\n");
    serial_print("SERIAL: Filesystem initialized\n");
    
    // Initialize network stack
    serial_print("SERIAL: Initializing network...\n");
    network_init();
    vga_print("Network stack initialized\n");
    serial_print("SERIAL: Network stack initialized\n");
    
    // Initialize UI system
    serial_print("SERIAL: Initializing UI...\n");
    ui_init();
    vga_print("UI system initialized\n");
    serial_print("SERIAL: UI system initialized\n");
    
    // Start shell
    vga_print("Starting shell...\n\n");
    serial_print("SERIAL: Starting shell...\n");
    shell_init();
    serial_print("SERIAL: Shell initialized, entering main loop\n");
    
    // Main kernel loop
    while (1) {
        shell_run();
    }
}
