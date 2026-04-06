#include "pstring.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "testlib.h"
#include "types.h"

// Test Variables

static char *empty_string = "";
static char *test_string_1 = "Test String";
static char *test_string_2 = "Another Test String";
static char *test_string_abc = "abc";
static char *test_string_xyz = "xyz";
static char *test_string_text = "Some Text";
static char *test_string_words = "Some Words";

// String Manipulation Tests

void pstrcpy_string() {
    char *expected_string = test_string_1;
    char *actual_string;
    char dest[20];

    actual_string = pstrcpy(dest, test_string_1);

    assert_continue(!strcmp(actual_string, expected_string));
}

void pstrcpy_empty_string() {
    char *expected_string = empty_string;
    char *actual_string;
    char dest[1];

    actual_string = pstrcpy(dest, empty_string);

    assert_continue(!strcmp(actual_string, expected_string));
}

void pstrncpy_string() {
    char *expected_string = test_string_2;
    char *actual_string;
    char dest[20];

    actual_string = pstrncpy(dest, test_string_2, /* size */ 20);

    assert_continue(!strcmp(actual_string, expected_string));
}

void pstrncpy_substring_short() {
    char *expected_string = "Another";
    char *actual_string;
    char dest[20];
    strcpy(dest, test_string_1);

    actual_string = pstrncpy(dest, test_string_2, 7);

    assert_continue(!strcmp(actual_string, expected_string));
}

void pstrncpy_substring_long() {
    char *expected_string = "Another Test Str";
    char *actual_string;
    char dest[20];
    strcpy(dest, test_string_1);

    actual_string = pstrncpy(dest, test_string_2, 16);

    assert_continue(!strcmp(actual_string, expected_string));
}

void pstrncpy_empty_string() {
    char *expected_string = empty_string;
    char *actual_string;
    char dest[20];
    strcpy(dest, test_string_1);

    actual_string = pstrncpy(dest, empty_string, 1);

    assert_continue(!strcmp(actual_string, expected_string));
}

void pstrcat_string() {
    char *expected_string = "abcxyz";
    char *actual_string;
    char dest[8];
    strcpy(dest, test_string_abc);

    actual_string = pstrcat(dest, test_string_xyz);

    assert_continue(!strcmp(actual_string, expected_string));
}

void pstrcat_empty_string() {
    char *expected_string = test_string_abc;
    char *actual_string;
    char dest[8];
    strcpy(dest, test_string_abc);

    actual_string = pstrcat(dest, empty_string);

    assert_continue(!strcmp(actual_string, expected_string));
}

void pstrncat_string() {
    char *expected_string = "abcxyz";
    char *actual_string;
    char dest[8];
    strcpy(dest, test_string_abc);

    actual_string = pstrncat(dest, test_string_xyz, 3);

    assert_continue(!strcmp(actual_string, expected_string));
}

void pstrncat_substring() {
    char *expected_string = "abcTest";
    char *actual_string;
    char dest[8];
    strcpy(dest, test_string_abc);

    actual_string = pstrncat(dest, test_string_1, 4);

    assert_continue(!strcmp(actual_string, expected_string));
}

void pstrncat_empty_string() {
    char *expected_string = test_string_abc;
    char *actual_string;
    char dest[8];
    strcpy(dest, test_string_abc);

    actual_string = pstrncat(dest, test_string_xyz, 0);

    assert_continue(!strcmp(actual_string, expected_string));
}

// String Examination Tests

void pstrlen_string() {
    isize expected_length = 11;
    isize actual_length;
    char *str = test_string_1;

    actual_length = pstrlen(str);

    assert_continue(actual_length == expected_length);
}

void pstrlen_empty_string() {
    isize expected_length = 0;
    isize actual_length;
    char *str = empty_string;

    actual_length = pstrlen(str);

    assert_continue(actual_length == expected_length);
}

void pstrcmp_same_string() {
    isize actual_result;
    char *lhs = test_string_1;
    char *rhs = test_string_1;
    isize expected_result = strcmp(lhs, rhs);

    actual_result = pstrcmp(lhs, rhs);

    assert_continue(actual_result == expected_result);
}

void pstrcmp_equal_strings() {
    isize actual_result;
    char *lhs = test_string_1;
    char *rhs = "Test String";
    isize expected_result = strcmp(lhs, rhs);

    actual_result = pstrcmp(lhs, rhs);

    assert_continue(actual_result == expected_result);
}

void pstrcmp_different_strings() {
    isize actual_result;
    char *lhs = test_string_abc;
    char *rhs = test_string_xyz;
    isize expected_result = strcmp(lhs, rhs);

    actual_result = pstrcmp(lhs, rhs);

    assert_continue(actual_result == expected_result);
}

void pstrncmp_equal_strings() {
    isize actual_result;
    char *lhs = test_string_text;
    char *rhs = test_string_words;
    usize n = 4;
    isize expected_result = strncmp(lhs, rhs, n);

    actual_result = pstrncmp(lhs, rhs, n);

    assert_continue(actual_result == expected_result);
}

void pstrncmp_different_strings() {
    isize actual_result;
    char *lhs = test_string_text;
    char *rhs = test_string_words;
    usize n = 7;
    isize expected_result = strncmp(lhs, rhs, n);

    actual_result = pstrncmp(lhs, rhs, n);

    assert_continue(actual_result == expected_result);
}

void pstrchr_char_in_string() {
    char *actual_result;
    char *str = test_string_1;
    char ch = 'S';
    char *expected_result = strchr(str, ch);

    actual_result = pstrchr(str, ch);

    assert_continue(actual_result == expected_result);
}

void pstrchr_char_not_in_string() {
    char *actual_result;
    char *str = test_string_1;
    char ch = 'p';
    char *expected_result = strchr(str, ch);

    actual_result = pstrchr(str, ch);

    assert_continue(actual_result == expected_result);
}

void pstrchr_null_terminator() {
    char *actual_result;
    char *str = test_string_1;
    char ch = '\0';
    char *expected_result = strchr(str, ch);

    actual_result = pstrchr(str, ch);

    assert_continue(actual_result == expected_result);
}

void pstrstr_substr_in_str() {
    char *actual_result;
    char *str = test_string_1;
    char *substr = "Str";
    char *expected_result = strstr(str, substr);

    actual_result = pstrstr(str, substr);

    assert_continue(actual_result == expected_result);
}

void pstrstr_substr_not_in_str() {
    char *actual_result;
    char *str = test_string_1;
    char *substr = "zyx";
    char *expected_result = strstr(str, substr);

    actual_result = pstrstr(str, substr);

    assert_continue(actual_result == expected_result);
}

void pstrstr_substr_empty() {
    char *actual_result;
    char *str = test_string_1;
    char *substr = empty_string;
    char *expected_result = strstr(str, substr);

    actual_result = pstrstr(str, substr);

    assert_continue(actual_result == expected_result);
}

// Memory Manipulation Tests

void pmemchr_found() {
    void *actual;
    char *str = test_string_2;
    char ch = 'S';
    void *expected = memchr(str, ch, strlen(str));

    actual = pmemchr(str, ch, strlen(str));

    assert_continue(actual == expected);
}

void pmemchr_not_found() {
    void *actual;
    char *str = test_string_abc;
    char ch = 'z';
    void *expected = memchr(str, ch, strlen(str));

    actual = pmemchr(str, ch, strlen(str));

    assert_continue(actual == expected);
}

void pmemcmp_equal() {
    isize actual_result;
    char *s1 = test_string_1;
    char *s2 = "Test String";
    int expected = memcmp(s1, s2, strlen(s1));

    actual_result = pmemcmp(s1, s2, strlen(s1));

    assert_continue(actual_result == expected);
}

void pmemcmp_not_equal() {
    isize actual;
    char *s1 = test_string_abc;
    char *s2 = test_string_xyz;
    int expected = memcmp(s1, s2, 3);

    actual = pmemcmp(s1, s2, 3);

    assert_continue(actual == expected);
}

void pmemset_fill() {
    char buf[10];
    char expected[10];
    memset(expected, 0xAA, 10);

    pmemset(buf, 0xAA, 10);

    assert_continue(!memcmp(buf, expected, 10));
}

void pmemcpy_copy() {
    char src[] = "Hello";
    char dest[10] = {0};
    char expected[10] = {0};
    memcpy(expected, src, 6);

    pmemcpy(dest, src, 6);

    assert_continue(!memcmp(dest, expected, 6));
}

int main() {
    // String Manipulation Tests
    test_handler("pstrcpy valid string", pstrcpy_string);
    test_handler("pstrcpy empty string", pstrcpy_empty_string);

    test_handler("pstrncpy valid string", pstrncpy_string);
    test_handler("pstrncpy valid substring short", pstrncpy_substring_short);
    test_handler("pstrncpy valid substring long", pstrncpy_substring_long);
    test_handler("pstrncpy empty string", pstrncpy_empty_string);

    test_handler("pstrcat append string", pstrcat_string);
    test_handler("pstrcat append empty string", pstrcat_empty_string);

    test_handler("pstrncat append string", pstrncat_string);
    test_handler("pstrncat append substring", pstrncat_substring);
    test_handler("pstrncat append empty string", pstrncat_empty_string);

    // String Examination Tests
    test_handler("pstrlen valid string", pstrlen_string);
    test_handler("pstrlen empty string", pstrlen_empty_string);

    test_handler("pstrcmp same string", pstrcmp_same_string);
    test_handler("pstrcmp equal strings", pstrcmp_equal_strings);
    test_handler("pstrcmp different strings", pstrcmp_different_strings);

    test_handler("pstrncmp equal strings", pstrncmp_equal_strings);
    test_handler("pstrncmp different strings", pstrncmp_different_strings);

    test_handler("pstrchr character in string", pstrchr_char_in_string);
    test_handler("pstrchr character not in string", pstrchr_char_not_in_string);
    test_handler("pstrchr null terminator", pstrchr_null_terminator);

    test_handler("pstrstr substring in string", pstrstr_substr_in_str);
    test_handler("pstrstr substring not in string", pstrstr_substr_not_in_str);
    test_handler("pstrstr substring is empty", pstrstr_substr_empty);

    // Memory Manipulation Tests
    test_handler("pmemchr character found", pmemchr_found);
    test_handler("pmemchr character not found", pmemchr_not_found);

    test_handler("pmemcmp equal", pmemcmp_equal);
    test_handler("pmemcmp not equal", pmemcmp_not_equal);

    test_handler("pmemset fill buffer", pmemset_fill);
    test_handler("pmemcpy copy buffer", pmemcpy_copy);

    // Output
    test_output(__FILE__);
}