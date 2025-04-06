#ifndef MEM_H_
#define MEM_H_

#include "lib/types.h"
#include "boot/multiboot.h"

void mem_init();
void map_pages(void *vaddr, u32 paddr, u32 count);
void alloc_pages(void *vaddr, u32 count);

void *kernel_alloc(u32 pages);
void *kernel_realloc(void *ptr, u32 pages);
void kernel_free(void *ptr);

void debug_heap(u32 pages);

#endif // MEM_H_
