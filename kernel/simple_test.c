#include "../drivers/vga.h"
#include "../include/string.h"
#include "log.h"

// Test case structure
typedef struct {
    const char* name;
    bool (*test_func)(void);
    bool passed;
} test_case_t;

// Forward declarations
bool test_strlen(void);
bool test_strcpy(void);
bool test_strcmp(void);
bool test_memset(void);
bool test_memcpy(void);

// Test cases
test_case_t tests[] = {
    {"strlen", test_strlen, false},
    {"strcpy", test_strcpy, false},
    {"strcmp", test_strcmp, false},
    {"memset", test_memset, false},
    {"memcpy", test_memcpy, false},
};

// Test implementations
bool test_strlen(void) {
    return strlen("") == 0 && 
           strlen("test") == 4 &&
           strlen("hello world") == 11;
}

bool test_strcpy(void) {
    char dest[32];
    strcpy(dest, "test");
    return strcmp(dest, "test") == 0;
}

bool test_strcmp(void) {
    return strcmp("", "") == 0 &&
           strcmp("a", "a") == 0 &&
           strcmp("a", "b") < 0 &&
           strcmp("b", "a") > 0;
}

bool test_memset(void) {
    char buf[16];
    memset(buf, 'A', sizeof(buf));
    for (size_t i = 0; i < sizeof(buf); i++) {
        if (buf[i] != 'A') return false;
    }
    return true;
}

bool test_memcpy(void) {
    char src[16] = "test";
    char dest[16];
    memcpy(dest, src, sizeof(src));
    return strcmp(src, dest) == 0;
}

// Test runner
void run_tests(void) {
    log_init();
    log_info("=== Kernel Test Runner Started ===");
    log_info("");
    
    size_t passed = 0;
    size_t total = sizeof(tests) / sizeof(tests[0]);
    
    for (size_t i = 0; i < total; i++) {
        tests[i].passed = tests[i].test_func();
        if (tests[i].passed) {
            passed++;
            vga_print("[PASS] ");
        } else {
            vga_print("[FAIL] ");
        }
        vga_print(tests[i].name);
        vga_print("\n");
    }
    
    log_info("=== Test Results ===");
    log_info("Tests completed. Analyzing results...");
    
    vga_print("\n=== Results ===\n");
    vga_print("Passed: ");
    // Simple number printing
    if (passed == total) {
        vga_print("ALL");
    } else {
        char num_str[32];
        char* p = num_str;
        size_t n = passed;
        do {
            *p++ = '0' + (n % 10);
            n /= 10;
        } while (n > 0);
        while (p > num_str) {
            vga_putchar(*--p);
        }
    }
    vga_print(" / ");
    // Print total
    {
        char num_str[32];
        char* p = num_str;
        size_t n = total;
        do {
            *p++ = '0' + (n % 10);
            n /= 10;
        } while (n > 0);
        while (p > num_str) {
            vga_putchar(*--p);
        }
    }
    vga_print(" tests passed\n\n");
    
    if (passed == total) {
        vga_print("ALL TESTS PASSED!\n");
        log_info("All tests completed successfully!");
    } else {
        vga_print("SOME TESTS FAILED!\n");
        log_warn("Some tests failed. Check output above.");
    }
    
    vga_print("\nSystem halted.");
    log_info("Test runner completed. System halted.");
}

// Kernel entry point
void kmain_tests(void) {
    run_tests();
    
    // Halt the system
    asm volatile ("hlt");
}
