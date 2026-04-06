#include "pstring.h"
#include "types.h"

// String Manipulation

char *pstrcpy(char *restrict dest, const char *restrict src) {
    char *ret = dest;
    while ((*dest++ = *src++))
        ;
    return ret;
}

char *pstrncpy(char *restrict dest, const char *restrict src, usize count) {
    char *ret = dest;
    while (count && (*dest++ = *src++))
        count--;
    while (count--)
        *dest++ = '\0';
    *dest = '\0';
    return ret;
}

char *pstrcat(char *restrict dest, const char *restrict src) {
    char *ret = dest;
    while (*dest)
        dest++;
    while ((*dest++ = *src++))
        ;
    return ret;
}

char *pstrncat(char *restrict dest, const char *restrict src, usize count) {
    char *ret = dest;
    while (*dest)
        dest++;
    while (count-- && *src)
        *dest++ = *src++;
    *dest = '\0';
    return ret;
}

// String Examination

usize pstrlen(const char *restrict str) {
    usize size = 0;
    while (*str++)
        size++;
    return size;
}

isize pstrcmp(const char *lhs, const char *rhs) {
    while (*lhs && (*lhs == *rhs)) {
        lhs++;
        rhs++;
    }
    return (unsigned char) (*lhs) - (unsigned char) (*rhs);
}

isize pstrncmp(const char *lhs, const char *rhs, usize count) {
    while (count--) {
        if (*lhs != *rhs)
            return (unsigned char) (*lhs) - (unsigned char) (*rhs);
        lhs++;
        rhs++;
    }
    return 0;
}

char *pstrchr(const char *str, char ch) {
    while (*str) {
        if (*str == ch)
            return (char *) str;
        str++;
    }
    return (ch == '\0') ? (char *) str : NULL;
}

char *pstrstr(const char *str, const char *substr) {
    if (!*substr)
        return (char *) str;

    for (; *str; str++) {
        const char *str_temp = str;
        const char *substr_temp = substr;
        while (*str_temp && *substr_temp && *str_temp == *substr_temp) {
            str_temp++;
            substr_temp++;
        }
        if (!*substr_temp)
            return (char *) str;
    }
    return NULL;
}

// Memory Manipulation

void *pmemchr(const void *ptr, char ch, usize count) {
    const unsigned char *p = ptr;
    while (count--) {
        if (*p == (unsigned char) ch)
            return (void *) p;
        p++;
    }
    return NULL;
}

isize pmemcmp(const void *lhs, const void *rhs, usize count) {
    const unsigned char *l = lhs, *r = rhs;
    while (count--) {
        if (*l != *r)
            return *l - *r;
        l++;
        r++;
    }
    return 0;
}

void pmemset(void *ptr, char ch, usize count) {
    unsigned char *p = ptr;
    while (count--)
        *p++ = (unsigned char) ch;
}

void pmemcpy(void *restrict dest, void *restrict src, usize count) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    while (count--)
        *d++ = *s++;
    return;
}
