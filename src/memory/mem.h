#ifndef MEM_H_
#define MEM_H_

#include "lib/types.h"
#include "boot/multiboot.h"

#define PAGE_SIZE 4096

#define PAGE_USER_MODE   4 // Can user access?
#define PAGE_WRITABLE    2 // Can user write?
#define PAGE_GLOBAL    512 // Flush from TLB on CR3 reload?

extern void load_page_dir(u32 paddr);

u32 size_in_pages(u32 size_in_bytes);
u32 align_next_frame(u32 paddr);
void *align_next_page(void *vaddr);

void mem_init();
void map_pages(void *vaddr, u32 paddr, u16 flags, u32 count);

u32 new_page_dir();

void alloc_pages(void *vaddr, u16 flags, u32 count);

void *kernel_alloc(u32 pages);
void *kernel_realloc(void *ptr, u32 pages);
void kernel_free(void *ptr);

void debug_heap(u32 pages);

#endif // MEM_H_
