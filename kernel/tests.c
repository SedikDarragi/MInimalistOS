#include "test.h"
#include "string_tests.h"

void run_all_tests(void) {
    // Run string tests
    run_test_suite(&string_test_suite);
    
    // Add more test suites here as they're created
}

// Main test entry point
void kmain_tests(void) {
    vga_clear();
    vga_print("Minimalist OS Test Runner\n");
    vga_print("========================\n\n");
    
    run_all_tests();
    
    vga_print("\nAll tests completed.\n");
    
    // Halt the system
    asm volatile ("hlt");
}
