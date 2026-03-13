#include "serial.h"
#include "../kernel/log.h"
#include "../kernel/io.h"

// Serial port I/O ports
#define SERIAL_COM1_BASE    0x3F8
#define SERIAL_DATA_PORT(base)      (base)
#define SERIAL_FIFO_CMD_PORT(base)   (base + 2)
#define SERIAL_LINE_CMD_PORT(base)  (base + 3)
#define SERIAL_MODEM_CMD_PORT(base) (base + 4)
#define SERIAL_LINE_STATUS_PORT(base) (base + 5)

// Serial port configuration
#define SERIAL_LINE_ENABLE_DLAB     0x80

// Serial port baud rate divisor for 38400 baud
#define SERIAL_BAUD_DIVISOR        3

// Initialize serial port for debugging
int serial_init(void) {
    // Disable interrupts during initialization
    outb(SERIAL_COM1_BASE + 1, 0x00);
    
    // Enable DLAB (set baud rate divisor)
    outb(SERIAL_LINE_CMD_PORT(SERIAL_COM1_BASE), SERIAL_LINE_ENABLE_DLAB);
    
    // Set divisor to 3 (lo byte) for 38400 baud
    outb(SERIAL_DATA_PORT(SERIAL_COM1_BASE), SERIAL_BAUD_DIVISOR & 0xFF);
    
    // Set divisor to 3 (hi byte) for 38400 baud
    outb(SERIAL_COM1_BASE + 1, (SERIAL_BAUD_DIVISOR >> 8) & 0xFF);
    
    // 8 bits, no parity, one stop bit
    outb(SERIAL_LINE_CMD_PORT(SERIAL_COM1_BASE), 0x03);
    
    // Enable FIFO, clear them, with 14-byte threshold
    outb(SERIAL_FIFO_CMD_PORT(SERIAL_COM1_BASE), 0xC7);
    
    // IRQs enabled, RTS/DSR set
    outb(SERIAL_MODEM_CMD_PORT(SERIAL_COM1_BASE), 0x0B);
    
    // The loopback test was previously disabled but still set the modem
    // control register, which can cause hangs in QEMU. We are removing
    // the loopback-related writes entirely. The 0x0B write above is sufficient.
    
    return 1; // Success
}

// Check if transmit buffer is empty
int serial_is_transmit_empty(uint16_t com) {
    return inb(SERIAL_LINE_STATUS_PORT(com)) & 0x20;
}

// Write character to serial port
void serial_putchar(uint16_t com, char c) {
    // Add timeout to prevent infinite hang if serial port is stuck
    int timeout = 100000;
    while (serial_is_transmit_empty(com) == 0) {
        if (timeout-- <= 0) break;
    }
    
    outb(SERIAL_DATA_PORT(com), c);
}

// Write string to serial port
void serial_write(uint16_t com, const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\n') {
            serial_putchar(com, '\r'); // Carriage return before newline
        }
        serial_putchar(com, str[i]);
    }
}

// Check if data is available to read
int serial_is_data_ready(uint16_t com) {
    return inb(SERIAL_LINE_STATUS_PORT(com)) & 0x01;
}

// Read character from serial port
char serial_getchar(uint16_t com) {
    while (serial_is_data_ready(com) == 0);
    
    return inb(SERIAL_DATA_PORT(com));
}

// Debug output functions
void serial_debug(const char* message) {
    serial_write(SERIAL_COM1_BASE, "[DEBUG] ");
    serial_write(SERIAL_COM1_BASE, message);
    serial_putchar(SERIAL_COM1_BASE, '\n');
}

void serial_info(const char* message) {
    serial_write(SERIAL_COM1_BASE, "[INFO] ");
    serial_write(SERIAL_COM1_BASE, message);
    serial_putchar(SERIAL_COM1_BASE, '\n');
}

void serial_error(const char* message) {
    serial_write(SERIAL_COM1_BASE, "[ERROR] ");
    serial_write(SERIAL_COM1_BASE, message);
    serial_putchar(SERIAL_COM1_BASE, '\n');
}

// Print hexadecimal value to serial port
void serial_hex(uint16_t com, uint32_t value) {
    const char hex_chars[] = "0123456789ABCDEF";
    serial_putchar(com, '0');
    serial_putchar(com, 'x');
    
    for (int i = 7; i >= 0; i--) {
        uint32_t nibble = (value >> (i * 4)) & 0xF;
        serial_putchar(com, hex_chars[nibble]);
    }
}

// Print decimal value to serial port
void serial_decimal(uint16_t com, uint32_t value) {
    if (value == 0) {
        serial_putchar(com, '0');
        return;
    }
    
    char buffer[16];
    int i = 0;
    
    while (value > 0) {
        buffer[i++] = (value % 10) + '0';
        value /= 10;
    }
    
    // Print in reverse order
    for (int j = i - 1; j >= 0; j--) {
        serial_putchar(com, buffer[j]);
    }
}

// Kernel panic output to serial port
void serial_panic(const char* message) {
    serial_write(SERIAL_COM1_BASE, "\n=== KERNEL PANIC ===\n");
    serial_write(SERIAL_COM1_BASE, message);
    serial_write(SERIAL_COM1_BASE, "\nSystem halted.\n");
    
    // Flush any remaining data
    for (int i = 0; i < 1000; i++) {
        if (serial_is_transmit_empty(SERIAL_COM1_BASE)) {
            break;
        }
    }
}
