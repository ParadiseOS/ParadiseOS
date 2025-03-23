#ifndef MEM_H_
#define MEM_H_

#include "lib/types.h"
#include "boot/multiboot.h"

void mem_init();
void *kernel_alloc(u32 pages);

#endif // MEM_H_
