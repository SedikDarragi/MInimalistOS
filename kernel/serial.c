#include <stdint.h>

// Serial ports I/O ports
#define SERIAL_COM1_BASE 0x3F8

// I/O port functions
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Initialize the serial port
void serial_init() {
    outb(SERIAL_COM1_BASE + 1, 0x00);    // Disable all interrupts
    outb(SERIAL_COM1_BASE + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(SERIAL_COM1_BASE + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(SERIAL_COM1_BASE + 1, 0x00);    //                  (hi byte)
    outb(SERIAL_COM1_BASE + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(SERIAL_COM1_BASE + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(SERIAL_COM1_BASE + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

// Check if the transmit FIFO is empty
static int serial_is_transmit_empty() {
    return inb(SERIAL_COM1_BASE + 5) & 0x20;
}

// Write a character to the serial port
void serial_putchar(char c) {
    while (serial_is_transmit_empty() == 0);
    outb(SERIAL_COM1_BASE, c);
}

// Write a string to the serial port
void serial_puts(const char* str) {
    while (*str) {
        if (*str == '\n') {
            serial_putchar('\r');
        }
        serial_putchar(*str++);
    }
}
