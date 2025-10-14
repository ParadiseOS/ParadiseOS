#ifndef STRINGS_H_
#define STRINGS_H_

#include "types.h" // For u32, etc.


/**
 * @brief Writes a formatted string to a buffer, ensuring not to write more
 * than n characters (including the null terminator).
 *
 * @param buffer The buffer to write the formatted string to.
 * @param n The maximum number of characters to write to the buffer,
 * including the null terminator.
 * @param fmt The format string.
 * @param ... The variable arguments for the format string.
 * @return The total number of characters that would have been written
 * if the buffer had been large enough, not including the
 * null terminator.
 */
u32 snprintf(char *buffer, u32 n, const char *fmt, ...);

/**
 * @brief Compares the lexicongraphical value of two strings
 *
 * @param s1 The first buffer to be compared
 * @param s2 The second buffer to be compared
 * @return ret < 0 if s1 < s2, ret > 0 if s1 > s2 else ret = 0 (s1 = s2)
 * Behavior is undefined for NULL s1, s2
 */
i32 strcmp(const char *s1, const char* s2);



#endif // STRINGS_H_
