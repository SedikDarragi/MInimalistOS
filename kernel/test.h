#ifndef TEST_H
#define TEST_H

#include <stdint.h>
#include <stdbool.h>

// Test function prototype
typedef bool (*test_func_t)(void);

// Test case structure
typedef struct {
    const char* name;
    test_func_t func;
    bool passed;
} test_case_t;

// Test suite structure
typedef struct {
    const char* name;
    test_case_t* tests;
    unsigned int num_tests;
    unsigned int passed_tests;
} test_suite_t;

// Test assertion macros
#define TEST_ASSERT(expr) \
    do { \
        if (!(expr)) { \
            return false; \
        } \
    } while(0)

// Test suite runner
void run_test_suite(test_suite_t* suite);

// Helper function to print test results
void print_test_results(const test_suite_t* suite);

#endif // TEST_H
