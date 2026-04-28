#ifndef PSTRING_H_
#define PSTRING_H_

#include "types.h"

// String Manipulation

// Copies null-terminated strings src to dest
// Behavior is undefined if strings overlap or dest cant contain all of src
// Behavior is undefined is dest or src are not null-terminated
char *pstrcpy(char *restrict dest, const char *restrict src);

// Copies at most count characters of null-terminated strings src to dest
// Behavior is undefined if strings overlap or dest cant contain all of src
// Behavior is undefined is dest or src are not null-terminated
char *pstrncpy(char *restrict dest, const char *restrict src, usize count);

// Appends a copy of null-terminated string src to the end of dest
// Behavior is undefined if strings overlap or dest cant contain all of src
// Behavior is undefined is dest or src are not null-terminated
char *pstrcat(char *restrict dest, const char *restrict src);

// Appends at most count character of null-terminated string src to dest
// Behavior is undefined if strings overlap or dest cant contain all of src
// Behavior is undefined if dest or src are not null-terminated
char *pstrncat(char *restrict dest, const char *restrict src, usize count);

// String Examination

// Returns the length of the given string
// Up to and not including the first null character
// Behavior is undefined if str is not null-terminated
usize pstrlen(const char *str);

// Compares two null-terminated strings lexicographically
// The sign of the result is the sign of the difference between the values of
// the first pair of characters that differ in the strings being compared
// Behavior is undefined if lhs or rhs are not null-terminated
isize pstrcmp(const char *lhs, const char *rhs);

// Compares at most count characters in two strings lexicographically
// The sign of the result is the sign of the difference between the values of
// the first pair of characters that differ in the strings being compared
// Behavior is undefined when access occurs past lhs or rhs
// Behavior is undefined when either lhs or rhs is the null pointer
isize pstrncmp(const char *lhs, const char *rhs, usize count);

// Finds the first occurrence of the character ch in str
// Returns a pointer to the character in str or null pointer if ch isn't found
char *pstrchr(const char *str, char ch);

// Returns a pointer to the first occurrence of the substr in str or null
// Behavior si undefined if str or substr are not null-terminated
char *pstrstr(const char *str, const char *substr);

// Memory Manipulation

// Returns a pointer to the first occurrence of ch in ptr up to count
// Behavior is undefined if access occurs beyond the end of ptr array
// Behavior is undefined if ptr is a null pointer
void *pmemchr(const void *ptr, char ch, usize count);

// Compares at most count characters in two pointers lexicographically
// The sign of the result is the sign of the difference between the values of
// the first pair of bytes that differ in the pointers being compared
// Behavior is undefined if access occurs beyond the end of lhs or rhs arrays
// Behavior is undefined if lhs or rhs is a null pointer
isize pmemcmp(const void *lhs, const void *rhs, usize count);

// Copies the value ch into each of the first count characters of ptr
// Behavior is undefined if access occurs beyond the ptr array
// Behavior is undefined if dest is a null pointer
void pmemset(void *ptr, char ch, usize count);

// Copies count bytes from src to dest
// Behavior is undefined if access occurs beyond the dest or src array
// Behavior is undefined if dest or src is a null pointer
// Behavior is undefined if dest and src overlap
void pmemcpy(void *restrict dest, void *restrict src, usize count);

#endif // PSTRING_H_