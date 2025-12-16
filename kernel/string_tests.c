#include "test.h"
#include "../include/string.h"

// Test string length function
bool test_strlen(void) {
    TEST_ASSERT(strlen("") == 0);
    TEST_ASSERT(strlen("a") == 1);
    TEST_ASSERT(strlen("test") == 4);
    return true;
}

// Test string copy function
bool test_strcpy(void) {
    char dest[32];
    TEST_ASSERT(strcmp(strcpy(dest, ""), "") == 0);
    TEST_ASSERT(strcmp(strcpy(dest, "test"), "test") == 0);
    return true;
}

// Test string compare function
bool test_strcmp(void) {
    TEST_ASSERT(strcmp("", "") == 0);
    TEST_ASSERT(strcmp("a", "a") == 0);
    TEST_ASSERT(strcmp("a", "b") < 0);
    TEST_ASSERT(strcmp("b", "a") > 0);
    return true;
}

// Test memory set function
bool test_memset(void) {
    char buf[16];
    memset(buf, 'A', sizeof(buf));
    for (size_t i = 0; i < sizeof(buf); i++) {
        TEST_ASSERT(buf[i] == 'A');
    }
    return true;
}

// Test memory copy function
bool test_memcpy(void) {
    char src[8] = "test";
    char dest[8];
    memcpy(dest, src, sizeof(src));
    TEST_ASSERT(memcmp(dest, src, sizeof(src)) == 0);
    return true;
}

// String test suite
test_case_t string_tests[] = {
    { "strlen", test_strlen },
    { "strcpy", test_strcpy },
    { "strcmp", test_strcmp },
    { "memset", test_memset },
    { "memcpy", test_memcpy },
};

test_suite_t string_test_suite = {
    "String Functions",
    string_tests,
    sizeof(string_tests) / sizeof(test_case_t),
    0
};
