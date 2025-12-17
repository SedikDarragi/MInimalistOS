#include "test.h"
#include "../drivers/vga.h"

void run_test_suite(test_suite_t* suite) {
    vga_print("\nRunning test suite: ");
    vga_print(suite->name);
    vga_print("\n");
    
    suite->passed_tests = 0;
    
    for (unsigned int i = 0; i < suite->num_tests; i++) {
        test_case_t* test = &suite->tests[i];
        vga_print("  [ ] ");
        vga_print(test->name);
        vga_print(" ");
        
        // Run the test
        test->passed = test->func();
        
        if (test->passed) {
            vga_print("[PASS]\n");
            suite->passed_tests++;
        } else {
            vga_print("[FAIL]\n");
        }
    }
    
    print_test_results(suite);
}

void print_test_results(const test_suite_t* suite) {
    vga_print("\nTest Results for ");
    vga_print(suite->name);
    vga_print(":\n");
    
    // Print numbers as strings (simplified for now)
    vga_print("  Total:   ");
    if (suite->num_tests < 10) vga_putchar('0' + suite->num_tests);
    else if (suite->num_tests < 100) {
        vga_putchar('0' + (suite->num_tests / 10));
        vga_putchar('0' + (suite->num_tests % 10));
    }
    vga_print("\n  Passed:  ");
    if (suite->passed_tests < 10) vga_putchar('0' + suite->passed_tests);
    else if (suite->passed_tests < 100) {
        vga_putchar('0' + (suite->passed_tests / 10));
        vga_putchar('0' + (suite->passed_tests % 10));
    }
    vga_print("\n  Failed:  ");
    unsigned int failed = suite->num_tests - suite->passed_tests;
    if (failed < 10) vga_putchar('0' + failed);
    else if (failed < 100) {
        vga_putchar('0' + (failed / 10));
        vga_putchar('0' + (failed % 10));
    }
    vga_print("\n\n");
}
