#ifndef UTIL_H_
#define UTIL_H_

#include "types.h"

extern u8 inb(u16 port);
extern void outb(u16 port, u8 data);

u16 get_pid_aid(u32 pid);
u16 get_pid_tid(u32 pid);

#endif // UTIL_H_
