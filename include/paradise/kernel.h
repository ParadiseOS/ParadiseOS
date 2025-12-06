#ifndef KERNEL_H_
#define KERNEL_H_

#include "types.h"

extern const u32 *kernel_stack;

extern const u32 _kernel_start_paddr;
extern const u32 _kernel_start_vaddr;
extern const u32 _kernel_end_paddr;
extern const u32 _kernel_end_vaddr;

extern const u32 kernel_start_paddr;
extern const void *kernel_start_vaddr;
extern const u32 kernel_end_paddr;
extern const void *kernel_end_vaddr;

#endif // KERNEL_H_
