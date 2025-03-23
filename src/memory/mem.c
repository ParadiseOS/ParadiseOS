#include "mem.h"
#include "lib/error.h"
#include "boot/multiboot.h"
#include "kernel/kernel.h"
#include "terminal/terminal.h"

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

// For now we will just have a heap that acts like a stack. Allocate addresses
// linearly and don't provide a way to free particular allocations.
u32 *heap_vaddr = NULL;

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

void free_frame(u32 paddr) {
    *free_list_head_pte = create_entry(paddr, PDE_USER_MODE | PDE_WRITABLE);
    invalidate_page(free_list_head);
    *free_list_head = free_frame_paddr;
    free_frame_paddr = paddr;
}

// Returns physical addresses that need to be mapped
u32 alloc_frame() {
    KERNEL_ASSERT(free_frame_paddr); // are we out of memory?
    u32 frame = free_frame_paddr;
    free_frame_paddr = *free_list_head;
    *free_list_head_pte = create_entry(free_frame_paddr, PDE_USER_MODE | PDE_WRITABLE);
    invalidate_page(free_list_head);
    return frame;
}

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

void map_pages(void *vaddr, u32 paddr, u32 count) {
    while (count--) {
        map_page(vaddr, paddr);
        paddr += 4096;
        vaddr += 4096;
    }
}

u32 align_next_page(u32 addr) {
    return (addr + 4095) / 4096 * 4096;
}

void init_heap() {
    heap_vaddr = (void *) align_next_page((u32) kernel_end_vaddr);
}

void *heap_next(u32 pages) {
    KERNEL_ASSERT(heap_vaddr < page_table_entries); // heap overflow?
    void *vaddr = heap_vaddr;
    heap_vaddr += pages * 4096;
    return vaddr;
}

void init_free_page() {
    void *addr = heap_next(1);
    u32 pdi = get_pd_index(addr);
    u32 pti = get_pt_index(addr);

    free_list_head_pte = &PTE(pdi, pti);
    KERNEL_ASSERT(entry_present(PDE(pdi)));
    KERNEL_ASSERT(!entry_present(*free_list_head_pte));
    free_list_head = addr;
}

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

    u32 kernel_page_count = (kernel_end_paddr - kernel_start_paddr + 4095) / 4096;
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

    for (u32 f = start_addr; f < end_addr; f += 4096) {
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

void mem_init() {
    asm ("cli"); // Probably don't want interrupts while doing this
    init_heap();
    init_free_page();
    init_frames();
    reinit_paging();
    asm ("sti");
}

void *kernel_alloc(u32 pages) {
    void *vaddr = heap_next(pages);

    while (pages--) {
        map_page(vaddr, alloc_frame());
        vaddr += 4096;
    }

    return vaddr;
}
