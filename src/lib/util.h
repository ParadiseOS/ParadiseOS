#ifndef UTIL_H_
#define UTIL_H_

#include "types.h"

#define TEMP_BUFFER_LENGTH 8192
// Buffer for miscellaneous operations
extern char temp_buffer[TEMP_BUFFER_LENGTH];

extern u8 inb(u16 port);
extern void outb(u16 port, u8 data);

#endif // UTIL_H_
