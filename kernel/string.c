#include <stddef.h>
#include <stdint.h>

// I/O port functions
void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void outw(uint16_t port, uint16_t value) {
    __asm__ volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void outl(uint16_t port, uint32_t value) {
    __asm__ volatile ("outl %0, %1" : : "a"(value), "Nd"(port));
}

uint32_t inl(uint16_t port) {
    uint32_t ret;
    __asm__ volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// A simple implementation of memset
void *memset(void *s, int c, size_t n) {
    unsigned char *p = s;
    while (n--) {
        *p++ = (unsigned char)c;
    }
    return s;
}

// String copy function
char *strcpy(char *dest, const char *src) {
    char *d = dest;
    while ((*d++ = *src++) != '\0');
    return dest;
}

// String comparison function
int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

// String concatenation function
char *strcat(char *dest, const char *src) {
    char *d = dest;
    while (*d) d++;
    while ((*d++ = *src++) != '\0');
    return dest;
}

// Memory copy function
void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

// String copy with limit function
char *strncpy(char *dest, const char *src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++)
        dest[i] = src[i];
    for ( ; i < n; i++)
        dest[i] = '\0';
    return dest;
}

// String comparison with limit function
int strncmp(const char *s1, const char *s2, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (s1[i] != s2[i]) {
            return (unsigned char)s1[i] - (unsigned char)s2[i];
        }
        if (s1[i] == '\0') {
            return 0;
        }
    }
    return 0;
}

// String length function
size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

// Integer to string conversion function
char* itoa(int value, char* str, int base) {
    char *ptr = str;
    char *ptr1 = str, tmp_char;
    int tmp_value;
    
    if (base < 2 || base > 36) {
        *str = '\0';
        return str;
    }

    if (value < 0 && base == 10) {
        *ptr++ = '-';
        ptr1++;
    }

    tmp_value = value;
    do {
        int rem = tmp_value % base;
        if (rem < 0) rem = -rem;
        *ptr++ = (rem < 10) ? (rem + '0') : (rem - 10 + 'a');
        tmp_value /= base;
    } while (tmp_value != 0);
    
    *ptr-- = '\0';
    while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
    
    return str;
}

// Memory comparison function
int memcmp(const void* ptr1, const void* ptr2, size_t size) {
    const unsigned char* p1 = ptr1;
    const unsigned char* p2 = ptr2;
    
    for (size_t i = 0; i < size; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] - p2[i];
        }
    }
    return 0;
}

// Interrupt control functions
void enable_interrupts(void) {
    __asm__ volatile ("sti");
}

void disable_interrupts(void) {
    __asm__ volatile ("cli");
}
