#include "mem.h"
#include "lib/error.h"
#include "boot/multiboot.h"
#include "kernel/kernel.h"
#include "terminal/terminal.h"

extern u32 page_directory_start;

u32 *page_directory = &page_directory_start;

#define PDE_USER_MODE 4 // Can user access?
#define PDE_WRITABLE  2 // Can user write?
                        //
#define PTE_USER_MODE   4 // Can user access?
#define PTE_WRITABLE    2 // Can user write?
#define PTE_GLOBAL    512 // TLB flush on CR3 reload?

void *free_frame = NULL;

// Will return a pointer to a frame
void *frame_alloc() {
    KERNEL_ASSERT(free_frame != NULL); // are we out of memory?
    void *frame = free_frame;
    free_frame = (void *) *((u32 *) frame);
    return frame; // NOTE: we leak the address of the next free frame
}

void frame_free(void *addr) {
    void *last = free_frame;
    free_frame = addr;
    *((u32 *) free_frame) = (u32) last;
}

u32 create_entry(void *addr, u8 flags) {
    return ((u32) addr & 0xFFFFF000) | flags | 1;
}

void init_page_directory(u32 *dir) {
    for (u32 i = 0; i < 1024; ++i) {
        dir[i] = 0; // the present flag gets cleared
    }

    // TODO: make sure we set WP bit in CR0 correctly
    dir[1023] = create_entry(dir, PDE_WRITABLE);
}

void init_page_table(u32 *table) {
    for (u32 i = 0; i < 1024; ++i) {
        table[i] = 0; // the present flag gets cleared
    }
}

void init_paging() {
    init_page_directory(page_directory);
}

// map a page before paging has been enabled
void map_page_physical(u32 physical_addr, u32 virtual_addr) {
    u32 directory_offset = (virtual_addr >> 22) & 0x3ff;
    u32 table_offset = (virtual_addr >> 12) & 0x3ff;

    if (!page_directory[directory_offset]) { // No page table exists yet
        u32 *new_table = frame_alloc();
        init_page_table(new_table);
        // for now the kernel is accessible from user mode
        page_directory[directory_offset] = create_entry(new_table, PDE_WRITABLE | PDE_USER_MODE);
    }

    u32 *table = (u32 *) (page_directory[directory_offset] & 0xFFFFF000);

    KERNEL_ASSERT(!table[table_offset]); // Page already mapped

    table[table_offset] = create_entry((void *) physical_addr, PTE_WRITABLE | PDE_USER_MODE);
}

void map_pages_physical(u32 physical_addr, u32 virtual_addr, u32 count) {
    while (count--) {
        map_page_physical(physical_addr, virtual_addr);
        physical_addr += 4096;
        virtual_addr += 4096;
    }
}

u32 align_next_page(u32 addr) {
    return (addr + 4095) / 4096 * 4096;
}

// Extends free frame list using a given range of frames
void init_frame(u32 start_addr, u32 end_addr) {
    start_addr = align_next_page(start_addr);

    for (u32 f = start_addr; f < end_addr; f += 4096) {
        u32 *next_frame = (u32 *) f;
        *next_frame = (u32) free_frame;
        free_frame = next_frame;
    }
}

// Initializes free frame list using all available frames
void init_frames() {
    MMapEntry *entries = multiboot_info->mmap_addr;

    KERNEL_ASSERT(multiboot_info->flags & MB_FLAG_MMAP);

    for (u32 i = 0; i < multiboot_info->mmap_length / sizeof (MMapEntry); i++) {
        if (entries[i].type == MMAP_AVAILABLE && entries[i].base_addr_lo >= (u32) kernel_start_addr) {
            if (entries[i].base_addr_lo == (u32) kernel_start_addr)
                init_frame((u32) kernel_end_addr, entries[i].base_addr_lo + entries[i].length_lo);
            else
                init_frame(entries[i].base_addr_lo, entries[i].base_addr_lo + entries[i].length_lo);
        }
    }
}
