#ifndef MEM_H_
#define MEM_H_

#include "boot/multiboot.h"
#include "heap.h"
#include "lib/error.h"
#include "lib/types.h"

#define PAGE_SIZE 4096

#define PAGE_USER_MODE 4   // Can user access?
#define PAGE_WRITABLE  2   // Can user write?
#define PAGE_GLOBAL    512 // Flush from TLB on CR3 reload?

typedef struct {
    u32 addr;
    u32 len;
} PhysicalMap;

extern void fpu_save(void *fpu_regs);
extern void fpu_restore(void *fpu_regs);

u32 get_paddr(u32 entry);
u32 get_entry(void *vaddr);
u16 get_flags(u32 entry);
void set_page_dir(u32 paddr);
u32 get_page_dir();

bool entry_present(u32 entry);

u32 size_in_pages(u32 size_in_bytes);
u32 align_next_frame(u32 paddr);
void *align_next_page(void *vaddr);
bool is_page_aligned(void *ptr);

void print_frame_usage();

void mem_init();
u32 alloc_frame();
RESULT map_page(void *vaddr, u32 paddr, u16 flags);
void map_pages(void *vaddr, u32 paddr, u16 flags, u32 count);
void swap_page_frames(void *vaddr1, void *vaddr2);

u32 new_page_dir();

void alloc_page(void *vaddr, u16 flags);
void alloc_pages(void *vaddr, u16 flags, u32 count);
void free_page(void *vaddr);
void free_pages(void *vaddr, u32 count);

// Reclaims a frame of physical memory.
void free_frame(u32 paddr);

void *mem_alloc(Heap *heap, u32 pages, u16 flags);
void *mem_realloc(Heap *heap, void *ptr, u32 pages);
void mem_free(Heap *heap, void *ptr);

void *kernel_alloc(u32 pages);
void *kernel_realloc(void *ptr, u32 pages);
void kernel_free(void *ptr);

void debug_heap(u32 pages);

void *validate_user_readable(u32 vaddr);
void *validate_user_writable(u32 vaddr);

#endif // MEM_H_
