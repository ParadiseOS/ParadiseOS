#ifndef KERNEL_H_
#define KERNEL_H_

#include "lib/types.h"

extern const u32 *kernel_stack;

extern const u32 _kernel_start;
extern const u32 _kernel_end;

extern const void *kernel_start_addr;
extern const void *kernel_end_addr;

#endif // KERNEL_H_
