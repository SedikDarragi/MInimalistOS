#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdint.h>

// String functions
size_t strlen(const char* str);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);

// Memory functions
void* memset(void* ptr, int value, size_t num);
void* memcpy(void* dest, const void* src, size_t n);

// Conversion functions
char* itoa(int value, char* str, int base);

#endif
