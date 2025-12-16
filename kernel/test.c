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
    vga_print("  Total:   ");
    vga_print_uint(suite->num_tests);
    vga_print("\n  Passed:  ");
    vga_print_uint(suite->passed_tests);
    vga_print("\n  Failed:  ");
    vga_print_uint(suite->num_tests - suite->passed_tests);
    vga_print("\n\n");
}
