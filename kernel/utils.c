#include "kernel.h"

// Simple memory allocator
static char heap[65536]; // 64KB heap
static size_t heap_pos = 0;

void* kmalloc(size_t size) {
    if (heap_pos + size >= sizeof(heap)) {
        return 0; // Out of memory
    }
    void* ptr = &heap[heap_pos];
    heap_pos += size;
    return ptr;
}

void kfree(void* ptr) {
    // Simple allocator - no free implementation
    (void)ptr;
}

// String functions
int strlen(const char* str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

int strcmp(const char* str1, const char* str2) {
    while (*str1 && *str1 == *str2) {
        str1++;
        str2++;
    }
    return *str1 - *str2;
}

int strncmp(const char* str1, const char* str2, size_t n) {
    while (n-- && *str1 && *str1 == *str2) {
        str1++;
        str2++;
    }
    return n == (size_t)-1 ? 0 : *str1 - *str2;
}

char* strcpy(char* dest, const char* src) {
    char* orig_dest = dest;
    while ((*dest++ = *src++));
    return orig_dest;
}

char* strncpy(char* dest, const char* src, size_t n) {
    char* orig_dest = dest;
    while (n-- && (*dest++ = *src++));
    while (n--) *dest++ = 0;
    return orig_dest;
}

char* strcat(char* dest, const char* src) {
    char* orig_dest = dest;
    while (*dest) dest++;
    while ((*dest++ = *src++));
    return orig_dest;
}

// Memory functions
void* memset(void* ptr, int value, size_t size) {
    unsigned char* p = (unsigned char*)ptr;
    while (size--) *p++ = (unsigned char)value;
    return ptr;
}

void* memcpy(void* dest, const void* src, size_t size) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    while (size--) *d++ = *s++;
    return dest;
}

int memcmp(const void* ptr1, const void* ptr2, size_t size) {
    const unsigned char* p1 = (const unsigned char*)ptr1;
    const unsigned char* p2 = (const unsigned char*)ptr2;
    while (size--) {
        if (*p1 != *p2) return *p1 - *p2;
        p1++;
        p2++;
    }
    return 0;
}

// I/O functions
void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void outw(uint16_t port, uint16_t value) {
    __asm__ volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

uint16_t inw(uint16_t port) {
    uint16_t value;
    __asm__ volatile ("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void enable_interrupts(void) {
    __asm__ volatile ("sti");
}

void disable_interrupts(void) {
    __asm__ volatile ("cli");
}
