#ifndef KERNEL_H
#define KERNEL_H

// Basic types
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;

typedef unsigned int size_t;

#ifndef NULL
#define NULL ((void*)0)
#endif

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
