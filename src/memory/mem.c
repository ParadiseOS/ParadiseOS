#include "mem.h"
#include "lib/error.h"
#include "boot/multiboot.h"
#include "kernel/kernel.h"
#include "lib/libp.h"
#include "terminal/terminal.h"
#include "memory/heap.h"

#define LOWER_MEM_PAGE_COUNT 256
#define MAX_KERNEL_PAGE_COUNT 0x300

// virtual addresses are represented with pointers
// physical addresses are represented with integers

extern void invalidate_page(void *vaddr);
extern void flush_tlb();

u32 *page_directory_entries = (u32 *) 0xFFFFF000;
u32 *page_table_entries = (u32 *) 0xFFC00000;

#define PDE(pd_index) (page_directory_entries[pd_index])
#define PTE(pd_index, pt_index) (page_table_entries[(pd_index) * 1024 + (pt_index)])

#define PDE_USER_MODE 4 // Can user access?
#define PDE_WRITABLE  2 // Can user write?

#define PTE_USER_MODE   4 // Can user access?
#define PTE_WRITABLE    2 // Can user write?
#define PTE_GLOBAL    512 // TLB flush on CR3 reload?

u32 *free_list_head = NULL;
u32 *free_list_head_pte = NULL;
u32 free_frame_paddr = 0;

Heap heap;

u32 get_pd_index(void *vaddr) {
    return (u32) vaddr >> 22;
}

u32 get_pt_index(void *vaddr) {
    return ((u32) vaddr >> 12) & 0x3FF;
}

u32 get_paddr(u32 entry) {
    return entry & ~0xFFF;
}

u32 *get_page_table(u32 pd_index) {
   return &PTE(pd_index, 0);
}

bool entry_present(u32 entry) {
    return (entry & 1) != 0;
}

u32 get_entry(void *vaddr) {
    u32 pdi = get_pd_index(vaddr);
    if (!entry_present(PDE(pdi))) return 0;
    return PTE(pdi, get_pt_index(vaddr));
}

u32 create_entry(u32 paddr, u16 flags) {
    return (paddr & 0xFFFFF000) | flags | 1;
}

void init_page_directory(u32 *dir) {
    for (u32 i = 0; i < 1024; ++i) {
        dir[i] = 0; // the present flag gets cleared
    }

    dir[1023] = create_entry((u32) dir, PDE_USER_MODE | PDE_WRITABLE);
}

void init_page_table(u32 *table) {
    for (u32 i = 0; i < 1024; ++i) {
        table[i] = 0; // the present flag gets cleared
    }
}

// Reclaims a frame of physical memory.
void free_frame(u32 paddr) {
    *free_list_head_pte = create_entry(paddr, PDE_USER_MODE | PDE_WRITABLE);
    invalidate_page(free_list_head);
    *free_list_head = free_frame_paddr;
    free_frame_paddr = paddr;
}

// Allocates a new frame of physical memory. Returns physical addresses that
// need to be mapped.
u32 alloc_frame() {
    KERNEL_ASSERT(free_frame_paddr); // are we out of memory?
    u32 frame = free_frame_paddr;
    free_frame_paddr = *free_list_head;
    *free_list_head_pte = create_entry(free_frame_paddr, PDE_USER_MODE | PDE_WRITABLE);
    invalidate_page(free_list_head);
    return frame;
}

// Maps a page in virtual memory. Does not back it with a physical address.
void map_page(void *vaddr, u32 paddr) {
    u32 pdi = get_pd_index(vaddr);
    u32 pti = get_pt_index(vaddr);

    if (!entry_present(PDE(pdi))) { // No page table exists yet
        u32 new_table = alloc_frame();
        PDE(pdi) = create_entry(new_table, PDE_WRITABLE | PDE_USER_MODE);
        init_page_table(get_page_table(pdi));
    }

    KERNEL_ASSERT(!entry_present(PTE(pdi, pti)));
    PTE(pdi, pti) = create_entry(paddr, PTE_WRITABLE | PTE_USER_MODE);
}

// Unmaps a page from virtual memory and returns the frame address. The frame is
// NOT freed.
u32 unmap_page(void *vaddr) {
    u32 pdi = get_pd_index(vaddr);
    u32 pti = get_pt_index(vaddr);

    KERNEL_ASSERT(entry_present(PDE(pdi)));
    KERNEL_ASSERT(entry_present(PTE(pdi, pti)));

    u32 paddr = get_paddr(PTE(pdi, pti));
    PTE(pdi, pti) = 0;
    invalidate_page(vaddr);
    return paddr;
}

// Maps a series of pages to virtual memory.
void map_pages(void *vaddr, u32 paddr, u32 count) {
    while (count--) {
        map_page(vaddr, paddr);
        paddr += PAGE_SIZE;
        vaddr += PAGE_SIZE;
    }
}

// Unmaps a series of pages from virtual memory. Does NOT free the frames.
void unmap_pages(void *vaddr, u32 count) {
    while (count--) {
        unmap_page(vaddr);
        vaddr += PAGE_SIZE;
    }
}

// Allocates pages at a given virtual address. Also backs the memory with
// physical frames.
void alloc_pages(void *vaddr, u32 count) {
    while (count--) {
        map_page(vaddr, alloc_frame());
        vaddr += PAGE_SIZE;
    }
}

// Frees pages starting at a given virtual address. Also frees the underlying
// frames.
void free_pages(void *vaddr, u32 count) {
    while (count--) {
        free_frame(unmap_page(vaddr));
        vaddr += PAGE_SIZE;
    }
}

u32 align_next_page(u32 addr) {
    return (addr + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE;
}

// Initializes the page which represents the head of our list of free frames and
// allows us to manipulate it. This page will be placed right after the kernel.
void init_free_list_page() {
    void *addr = (void *) align_next_page((u32) kernel_end_vaddr);
    u32 pdi = get_pd_index(addr);
    u32 pti = get_pt_index(addr);

    free_list_head_pte = &PTE(pdi, pti);
    KERNEL_ASSERT(entry_present(PDE(pdi)));
    KERNEL_ASSERT(!entry_present(*free_list_head_pte));
    free_list_head = addr;
}

// Returns the physical address that a given virtual address maps to. The
// virtual address must be already mapped.
u32 query_paddr(void *vaddr) {
    u32 pdi = get_pd_index(vaddr);
    u32 pti = get_pt_index(vaddr);
    KERNEL_ASSERT(entry_present(PDE(pdi)));
    KERNEL_ASSERT(entry_present(PTE(pdi, pti)));
    return PTE(pdi, pti) & 0xFFFFF000;
}

// Transitions from basic paging structures of boot.s to ones that are
// completely in the c code
void reinit_paging() {
    // The strategy we will use here: use some random parts of the address space
    // to build our identity and kernel mappings. We will then transfer those
    // into the correct page directory entries. The original page directory will
    // be reused (since it has the same lifetime as the kernel) but the original
    // page table will be discarded. We are assuming that our entire kernel can
    // be mapped using only one page table. This assumption should be enforced
    // in the boot code.

    u32 kernel_page_count = (kernel_end_paddr - kernel_start_paddr + PAGE_SIZE - 1) / PAGE_SIZE;
    u32 kernel_offset = (u32) kernel_start_vaddr - kernel_start_paddr;

    // Two addresses which are guaranteed to allocate two new page tables and
    // create paging structures which will mirror what we need for the kernel.
    void *id_map_vaddr = (void *) (kernel_offset - (1 << 24));
    void *kernel_map_vaddr = (void *) (kernel_start_vaddr - (2 << 24));

    KERNEL_ASSERT(kernel_page_count <= MAX_KERNEL_PAGE_COUNT);

    map_pages(id_map_vaddr, 0, LOWER_MEM_PAGE_COUNT);
    map_pages(kernel_map_vaddr, kernel_start_paddr, kernel_page_count);

    u32 old_free_list_entry = *free_list_head_pte;

    u32 id_map_index_src = get_pd_index(id_map_vaddr);
    u32 id_map_index_dst = get_pd_index(0);
    u32 kernel_map_index_src = get_pd_index(kernel_map_vaddr);
    u32 kernel_map_index_dst = get_pd_index((void *) kernel_start_vaddr);

    PDE(id_map_index_dst) = create_entry(get_paddr(PDE(id_map_index_src)), PDE_USER_MODE | PDE_WRITABLE);
    PDE(kernel_map_index_dst) = create_entry(get_paddr(PDE(kernel_map_index_src)), PDE_USER_MODE | PDE_WRITABLE);

    PDE(id_map_index_src) = 0;
    PDE(kernel_map_index_src) = 0;

    flush_tlb();

    // All heap pages are lost so we manually recover the only one that was
    // allocated.
    *free_list_head_pte = old_free_list_entry;
}

// Extends free frame list using a given range of frames
void init_frame_region(u32 start_addr, u32 end_addr) {
    start_addr = align_next_page(start_addr);

    for (u32 f = start_addr; f < end_addr; f += PAGE_SIZE) {
        free_frame(f);
    }
}

// Initializes free frame list using all available frames
void init_frames() {
    MMapEntry *entries = multiboot_info->mmap_addr;

    KERNEL_ASSERT(multiboot_info->flags & MB_FLAG_MMAP);

    for (u32 i = 0; i < multiboot_info->mmap_length / sizeof (MMapEntry); i++) {
        if (entries[i].type == MMAP_AVAILABLE && entries[i].base_addr_lo >= (u32) kernel_start_paddr) {
            if (entries[i].base_addr_lo == (u32) kernel_start_paddr)
                init_frame_region((u32) kernel_end_paddr - 1, entries[i].base_addr_lo + entries[i].length_lo);
            else
                init_frame_region(entries[i].base_addr_lo, entries[i].base_addr_lo + entries[i].length_lo);
        }
    }
}

// Initializes the kernel heap, giving it all pages after the free list page and
// before the page tables.
void init_heap() {
    void *heap_addr = (void *) free_list_head + PAGE_SIZE;
    u32 page_count = ((void *) page_table_entries - heap_addr) / PAGE_SIZE;

    heap_init(&heap, heap_addr, page_count);
}

// Performs all operations required to initialize our kernel memory management.
void mem_init() {
    asm ("cli"); // Probably don't want interrupts while doing this
    init_free_list_page();
    init_frames();
    reinit_paging();
    init_heap();
    asm ("sti");
}

// Allocates a given number of pages on the heap. Empty allocations are not
// allowed.
void *kernel_alloc(u32 pages) {
    KERNEL_ASSERT(pages);
    void *vaddr = heap_alloc(&heap, pages);
    alloc_pages(vaddr, pages);
    return vaddr;
}

// Reallocates an existing allocation to a new size. This may require moving the
// pointer and copying the data so the new pointer is returned. If a different
// pointer is returned, the old pointer is invalidated.
void *kernel_realloc(void *ptr, u32 pages) {
    KERNEL_ASSERT(pages);

    void *new_ptr;
    u32 old_size = heap_realloc(&heap, ptr, &new_ptr, pages);

    if (new_ptr != ptr) {
        alloc_pages(new_ptr, pages);
        pmemcpy(new_ptr, ptr, old_size * PAGE_SIZE);
        free_pages(ptr, old_size);
    }
    else {
        if (old_size > pages) {
            free_pages(ptr + pages * PAGE_SIZE, old_size - pages);
        }
        else {
            alloc_pages(ptr + old_size * PAGE_SIZE, pages - old_size);
        }
    }

    return new_ptr;
}

// Frees an existing allocation.
void kernel_free(void *ptr) {
    u32 freed_count = heap_free(&heap, ptr);
    free_pages(ptr, freed_count);
}

// Ensures that heap pages are marked as used if and only if their corresponding
// page table entry is present. This function checks every page on the heap and
// thus should only be used for debugging purposes. We also print a
// representation of the usage of the first `pages` pages of the heap.
void debug_heap(u32 pages) {
    for (void *addr = heap.heap_start; addr < (void *) page_table_entries; addr += PAGE_SIZE) {
        KERNEL_ASSERT(heap_is_used(&heap, addr) == entry_present(get_entry(addr)));
    }

    heap_debug(&heap, pages);
}
