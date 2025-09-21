#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stddef.h>

// Common macros
#define UNUSED(x) (void)(x)

// Memory management constants
#define KERNEL_HEAP_START 0xC0000000
#define KERNEL_HEAP_END   0xFFBFFFFF
#define PAGE_SIZE         4096

// Memory management
void* kmalloc(size_t size);
void kfree(void* ptr);

// String functions
int strlen(const char* str);
int strcmp(const char* str1, const char* str2);
int strncmp(const char* str1, const char* str2, size_t n);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
char* strcat(char* dest, const char* src);

// Memory functions
void* memset(void* ptr, int value, size_t size);
void* memcpy(void* dest, const void* src, size_t size);
int memcmp(const void* ptr1, const void* ptr2, size_t size);

// I/O functions
void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);
void outw(uint16_t port, uint16_t value);
uint16_t inw(uint16_t port);

// Interrupt handling
void enable_interrupts(void);
void disable_interrupts(void);

#endif
