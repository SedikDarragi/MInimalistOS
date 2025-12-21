#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

// Serial port base addresses
#define SERIAL_COM1    0x3F8
#define SERIAL_COM2    0x2F8
#define SERIAL_COM3    0x3E8
#define SERIAL_COM4    0x2E8

// Function declarations
int serial_init(void);
int serial_is_transmit_empty(uint16_t com);
void serial_putchar(uint16_t com, char c);
void serial_write(uint16_t com, const char* str);
int serial_is_data_ready(uint16_t com);
char serial_getchar(uint16_t com);

// Debug functions
void serial_debug(const char* message);
void serial_info(const char* message);
void serial_error(const char* message);
void serial_hex(uint16_t com, uint32_t value);
void serial_decimal(uint16_t com, uint32_t value);
void serial_panic(const char* message);

#endif // SERIAL_H
