#ifndef PIO_H_
#define PIO_H_

#include "types.h"

// Prints a character to to the terminal
void pputc(const char c);

// Prints a null terminated string to the terminal
void pputs(const char *str);

// Prints n characters of a string to the terminal
void pputn(const char *string, u32 length);

// Prints formatted string to terminal
void pprintf(const char *fmt, ...);

#endif // PIO_H_