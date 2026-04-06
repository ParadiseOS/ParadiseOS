#include "testlib.h"
#include "types.h"

// Global Vars
int tests_run = 0;
int tests_failed = 0;
int test_failed = 0;

void test_handler(const char *test_name, test_func_t func) {
    tests_run++;
    test_failed = 0;

    func();

    if (test_failed)
        tests_failed++;

    test_print(test_name);
}

void test_output(const char *filename) {
#ifdef SIMPLE_PRINT
    if (tests_failed == 0) {
        printf("%s: PASS\n", filename);
    }
    else {
        printf("%s: FAIL\n", filename);
    }
#endif // SIMPLE_PRINT

#if defined(COMPACT_PRINT) || defined(VERBOSE_PRINT)
    int tests_passed = tests_run - tests_failed;
    printf("%s: %d passed, %d failed\n", filename, tests_passed, tests_failed);
#endif
}

void test_print(const char *func_name) {
#ifdef COMPACT_PRINT
    if (test_failed) {
        printf("[FAIL] %s\n", func_name);
    }
#endif // COMPACT_PRINT

#ifdef VERBOSE_PRINT
    if (test_failed) {
        printf("[FAIL] %s\n", func_name);
    }
    else {
        printf("[PASS] %s\n", func_name);
    }
#endif // VERBOSE_PRINT
}

void verbose_print(const char *func_name, const char *str) {
#ifdef VERBOSE_PRINT
    printf("%s: %s\n", func_name, str);
#endif // VERBOSE_PRINT
}