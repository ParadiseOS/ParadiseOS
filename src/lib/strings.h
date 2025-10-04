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
int snprintf(char *buffer, u32 n, const char *fmt, ...);

#endif // STRINGS_H_
