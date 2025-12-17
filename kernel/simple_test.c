#include "../drivers/vga.h"
#include "../include/string.h"

// Simple test functions
bool test_strlen(void) {
    return strlen("") == 0 && strlen("test") == 4;
}

bool test_strcpy(void) {
    char dest[32];
    strcpy(dest, "test");
    return strcmp(dest, "test") == 0;
}

bool test_memset(void) {
    char buf[16];
    memset(buf, 'A', sizeof(buf));
    for (size_t i = 0; i < sizeof(buf); i++) {
        if (buf[i] != 'A') return false;
    }
    return true;
}

// Main test entry point
void kmain_tests(void) {
    vga_clear();
    vga_print("=== Simple Kernel Tests ===\n\n");
    
    // Run tests
    vga_print("Test 1: strlen() ");
    if (test_strlen()) {
        vga_print("[PASS]\n");
    } else {
        vga_print("[FAIL]\n");
    }
    
    vga_print("Test 2: strcpy() ");
    if (test_strcpy()) {
        vga_print("[PASS]\n");
    } else {
        vga_print("[FAIL]\n");
    }
    
    vga_print("Test 3: memset() ");
    if (test_memset()) {
        vga_print("[PASS]\n");
    } else {
        vga_print("[FAIL]\n");
    }
    
    vga_print("\n=== Tests Complete ===\n");
    
    // Halt
    asm volatile ("hlt");
}
