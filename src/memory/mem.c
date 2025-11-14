#include "mem.h"
#include "boot/multiboot.h"
#include "kernel/kernel.h"
#include "lib/error.h"
#include "lib/libp.h"
#include "lib/logging.h"
#include "lib/util.h"
#include "memory/heap.h"
#include "process/processes.h"
#include "syscall/syscall.h"

#define LOWER_MEM_PAGE_COUNT  256
#define MAX_KERNEL_PAGE_COUNT 0x300 // last 3/4 of page table
#define KERNEL_PDI_START      0x300 // top 1/4 of memory
#define KERNEL_PDI_END        0x3FF // all but final self-map of page dir

// virtual addresses are represented with pointers
// physical addresses are represented with integers
// virtual addresses in user space are represented with integers

extern void invalidate_page(void *vaddr);
extern void flush_tlb();

extern u32 get_cr3();
extern void set_cr3(u32 paddr);

u32 *page_directory_entries = (u32 *) 0xFFFFF000;
u32 *page_table_entries = (u32 *) 0xFFC00000;

// clang-format off
#define PDE(pd_index) (page_directory_entries[pd_index])
#define PTE(pd_index, pt_index)                                                \
    (page_table_entries[(pd_index) * 1024 + (pt_index)])
// clang-format on

#define PDE_USER_MODE 4 // Can user access?
#define PDE_WRITABLE  2 // Can user write?

#define PTE_USER_MODE 4   // Can user access?
#define PTE_WRITABLE  2   // Can user write?
#define PTE_GLOBAL    512 // Flush from TLB on CR3 reload?

// Flags for memory mapping from a syscall. We don't have user mode flags here
// since the user can only map user-mode pages.
#define VIRT_MAP_WRITABLE 1

// Division of physical memory that is reserved
// for the kernel
#define KERNEL_MEM_SHARE_DIV 5

#define USER_RO_PAGES 1024 // Number of virtual pages for user read-only memory

u32 kernel_page_dir = 0;

u32 *free_list_head = NULL;
u32 *free_list_head_pte = NULL;
u32 free_frame_paddr = 0;
u32 used_frames = 0;
u32 total_frames = 0;

u32 current_page_dir; // Should mirror cr3

Heap kernel_heap;
Heap user_ro_heap;

u8 user_physical_map_buffer[PAGE_SIZE];
PhysicalMap *user_physical_map = (PhysicalMap *) user_physical_map_buffer;
u32 user_physical_map_len = 0;

typedef struct {
    u16 pdi;
    u16 pti;
    u32 entry;
} EntryInfo;

static u32 get_pd_index(void *vaddr) {
    return (u32) vaddr >> 22;
}

static u32 get_pt_index(void *vaddr) {
    return ((u32) vaddr >> 12) & 0x3FF;
}

u32 get_paddr(u32 entry) {
    return entry & ~0xFFF;
}

u16 get_flags(u32 entry) {
    return entry & 0xFFF;
}

__attribute__((warn_unused_result)) static u32 set_paddr(u32 entry, u32 paddr) {
    return get_flags(entry) | paddr;
}

// __attribute__((warn_unused_result))
// static u16 set_flags(u32 entry, u16 flags) {
//     return get_paddr(entry) | flags;
// }

u32 *get_page_table(u32 pd_index) {
    return &PTE(pd_index, 0);
}

bool entry_present(u32 entry) {
    return (entry & 1) != 0;
}

u32 get_entry(void *vaddr) {
    u32 pdi = get_pd_index(vaddr);
    if (!entry_present(PDE(pdi)))
        return 0;
    return PTE(pdi, get_pt_index(vaddr));
}

static u32 create_entry(u32 paddr, u16 flags) {
    return (paddr & 0xFFFFF000) | flags | 1;
}

static EntryInfo get_entry_info(void *vaddr) {
    return (EntryInfo) {
        .pdi = get_pd_index(vaddr),
        .pti = get_pt_index(vaddr),
        .entry = get_entry(vaddr),
    };
}

// updates the page tables based on an entry
static void flush_entry_info(EntryInfo e) {
    KERNEL_ASSERT(entry_present(PDE(e.pdi)));
    KERNEL_ASSERT(entry_present(PTE(e.pdi, e.pti)));

    PTE(e.pdi, e.pti) = e.entry;
}

void set_page_dir(u32 paddr) {
    if (paddr != current_page_dir) {
        set_cr3(paddr);
        current_page_dir = paddr;
    }
}

u32 get_page_dir() {
    return current_page_dir;
}

void init_page_table(u32 *table) {
    for (u32 i = 0; i < 1024; ++i) {
        table[i] = 0; // the present flag gets cleared
    }
}

// Reclaims a frame of physical memory.
void free_frame(u32 paddr) {
    --used_frames;
    *free_list_head_pte = create_entry(paddr, PTE_WRITABLE);
    invalidate_page(free_list_head);
    *free_list_head = free_frame_paddr;
    free_frame_paddr = paddr;
}

// Allocates a new frame of physical memory. Returns physical addresses that
// need to be mapped.
u32 alloc_frame() {
    KERNEL_ASSERT(free_frame_paddr); // are we out of memory?
    ++used_frames;
    u32 frame = free_frame_paddr;
    free_frame_paddr = *free_list_head;
    *free_list_head_pte = create_entry(free_frame_paddr, PTE_WRITABLE);
    invalidate_page(free_list_head);
    return frame;
}

void print_frame_usage() {
    printk(INFO, "Usage: %u/%u\n", used_frames, total_frames);
}

// Maps a page in virtual memory. Does not back it with a physical address.
// Non-zero return means that the page was already mapped.
RESULT map_page(void *vaddr, u32 paddr, u16 flags) {
    u32 pdi = get_pd_index(vaddr);
    u32 pti = get_pt_index(vaddr);

    if (!entry_present(PDE(pdi))) { // No page table exists yet
        u32 new_table = alloc_frame();
        // Every PDE must be writable, or else we won't be able to edit the
        // paging structures through the self map of the page directory. Non
        // writeable pages must be set at the PTE level. Since, additionally,
        // PDEs cannot be global, we only care about the user mode flag here.
        PDE(pdi) =
            create_entry(new_table, PDE_WRITABLE | (flags & PAGE_USER_MODE));
        init_page_table(get_page_table(pdi));
    }

    bool user_mode_requested = flags & PAGE_USER_MODE;
    bool pde_user_mode = get_flags(PDE(pdi)) & PAGE_USER_MODE;

    // If user mode is requested of the page, it must be present in the PDE.
    KERNEL_ASSERT(!user_mode_requested || pde_user_mode);
    if (entry_present(PTE(pdi, pti)))
        return true;

    PTE(pdi, pti) = create_entry(paddr, flags);
    return false;
}

// Unmaps a page from virtual memory and outputs the frame address to `paddr` if
// non-null. The frame is NOT freed. Returns non-zero if the page was not
// already mapped.
RESULT unmap_page(void *vaddr, u32 *entry) {
    EntryInfo info = get_entry_info(vaddr);

    if (!entry_present(info.entry))
        return true;

    if (entry)
        *entry = info.entry;

    info.entry = 0;
    flush_entry_info(info);
    invalidate_page(vaddr);

    return false;
}

// Swaps the backing frames of two already allocated pages. Permissions remain
// the same for each page.
void swap_page_frames(void *vaddr1, void *vaddr2) {
    EntryInfo info1 = get_entry_info(vaddr1);
    EntryInfo info2 = get_entry_info(vaddr2);

    u32 swapped_entry1 = set_paddr(info1.entry, get_paddr(info2.entry));
    u32 swapped_entry2 = set_paddr(info2.entry, get_paddr(info1.entry));

    info1.entry = swapped_entry1;
    info2.entry = swapped_entry2;

    flush_entry_info(info1);
    flush_entry_info(info2);

    invalidate_page(vaddr1);
    invalidate_page(vaddr2);
}

// Maps a series of pages to virtual memory.
void map_pages(void *vaddr, u32 paddr, u16 flags, u32 count) {
    while (count--) {
        KERNEL_ASSERT(!map_page(vaddr, paddr, flags));
        paddr += PAGE_SIZE;
        vaddr += PAGE_SIZE;
    }
}

// Unmaps a series of pages from virtual memory. Does NOT free the frames.
void unmap_pages(void *vaddr, u32 count) {
    while (count--) {
        KERNEL_ASSERT(!unmap_page(vaddr, NULL));
        vaddr += PAGE_SIZE;
    }
}

// Allocates page at given virtual address & backs memory with physical frame.
void alloc_page(void *vaddr, u16 flags) {
    KERNEL_ASSERT(!map_page(vaddr, alloc_frame(), flags));
}

// Allocates pages at a given virtual address. Also backs the memory with
// physical frames.
void alloc_pages(void *vaddr, u16 flags, u32 count) {
    while (count--) {
        KERNEL_ASSERT(!map_page(vaddr, alloc_frame(), flags));
        vaddr += PAGE_SIZE;
    }
}

// Frees page at a given virtual address & frees underlying frame
void free_page(void *vaddr) {
    u32 paddr;
    KERNEL_ASSERT(!unmap_page(vaddr, &paddr));
    free_frame(paddr);
}

// Frees pages starting at a given virtual address. Also frees the underlying
// frames.
void free_pages(void *vaddr, u32 count) {
    while (count--) {
        u32 entry;
        KERNEL_ASSERT(!unmap_page(vaddr, &entry));
        free_frame(get_paddr(entry));
        vaddr += PAGE_SIZE;
    }
}

u32 size_in_pages(u32 size_in_bytes) {
    return (size_in_bytes + PAGE_SIZE - 1) / PAGE_SIZE;
}

u32 align_next_frame(u32 paddr) {
    return (paddr + PAGE_SIZE) / PAGE_SIZE * PAGE_SIZE;
}

void *align_next_page(void *vaddr) {
    return (void *) align_next_frame((u32) vaddr);
}

bool is_page_aligned(void *ptr) {
    return ((u32) ptr << 20) == 0;
}

// Initializes the page which represents the head of our list of free frames and
// allows us to manipulate it. This page will be placed right after the kernel.
void init_free_list_page() {
    void *addr = align_next_page((void *) kernel_end_vaddr - 1);
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

    u32 page_count = size_in_pages(kernel_end_paddr - kernel_start_paddr);
    u32 kernel_offset = (u32) kernel_start_vaddr - kernel_start_paddr;

    // Two addresses which are guaranteed to allocate two new page tables and
    // create paging structures which will mirror what we need for the kernel.
    void *id_map_vaddr = (void *) (kernel_offset - (1 << 24));
    void *kernel_map_vaddr = (void *) (kernel_start_vaddr - (2 << 24));

    KERNEL_ASSERT(page_count <= MAX_KERNEL_PAGE_COUNT);

    map_pages(id_map_vaddr, 0, PAGE_WRITABLE, LOWER_MEM_PAGE_COUNT);
    map_pages(kernel_map_vaddr, kernel_start_paddr, PAGE_WRITABLE, page_count);

    u32 old_free_list_entry = *free_list_head_pte;

    u32 id_map_index_src = get_pd_index(id_map_vaddr);
    u32 id_map_index_dst = get_pd_index(0);
    u32 kernel_map_index_src = get_pd_index(kernel_map_vaddr);
    u32 kernel_map_index_dst = get_pd_index((void *) kernel_start_vaddr);

    PDE(id_map_index_dst) =
        create_entry(get_paddr(PDE(id_map_index_src)), PDE_WRITABLE);
    PDE(kernel_map_index_dst) =
        create_entry(get_paddr(PDE(kernel_map_index_src)), PDE_WRITABLE);

    PDE(id_map_index_src) = 0;
    PDE(kernel_map_index_src) = 0;

    flush_tlb();

    // All heap pages are lost so we manually recover the only one that was
    // allocated.
    *free_list_head_pte = old_free_list_entry;
}

// Extends free frame list using a given range of frames
void init_frame_region(u32 start_addr, u32 end_addr) {
    KERNEL_ASSERT(end_addr > start_addr);

    start_addr = align_next_frame(start_addr - 1);

    for (u32 f = start_addr; f < end_addr; f += PAGE_SIZE) {
        ++total_frames;
        ++used_frames;
        free_frame(f);
    }
}

// Initializes free frame list using all available frames
void init_frames() {
    MMapEntry *entries = multiboot_info->mmap_addr;
    u32 entries_len = multiboot_info->mmap_length / sizeof(*entries);

    KERNEL_ASSERT(multiboot_info->flags & MB_FLAG_MMAP);

    for (u32 i = 0; i < entries_len; i++) {
        KERNEL_ASSERT(entries[i].base_addr_hi == 0);
        KERNEL_ASSERT(entries[i].length_hi == 0);

        // NOTE: maybe make this syscall work for variable entry sizes
        KERNEL_ASSERT(entries[i].size == sizeof(MMapEntry) - sizeof(u32));

        bool mem_available =
            entries[i].type == MMAP_AVAILABLE &&
            entries[i].base_addr_lo >= (u32) kernel_start_paddr;

        u32 kernel_share = entries[i].length_lo / KERNEL_MEM_SHARE_DIV;

        // NOTE: if the kernel is not aligned to a mmap entry, this won't work.
        // Maybe that should be fixed.
        if (mem_available) {
            KERNEL_ASSERT(
                user_physical_map_len < PAGE_SIZE / sizeof(PhysicalMap)
            );

            if (entries[i].base_addr_lo == (u32) kernel_start_paddr) {
                user_physical_map[user_physical_map_len++] = (PhysicalMap) {
                    .addr = entries[i].base_addr_lo + kernel_share,
                    .len = entries[i].length_lo - kernel_share,
                };

                init_frame_region(
                    (u32) kernel_end_paddr,
                    entries[i].base_addr_lo + kernel_share
                );
            }
            else {
                user_physical_map[user_physical_map_len++] = (PhysicalMap) {
                    .addr = entries[i].base_addr_lo + kernel_share,
                    .len = entries[i].length_lo - kernel_share,
                };

                init_frame_region(
                    entries[i].base_addr_lo,
                    entries[i].base_addr_lo + kernel_share
                );
            }
        }
    }
}

// Initializes the kernel heap, giving it all pages after the free list page and
// before the page tables.
void init_heap() {
    void *heap_addr = (void *) free_list_head + PAGE_SIZE;
    u32 page_count = ((void *) page_table_entries - heap_addr) / PAGE_SIZE;

    // If this fails then we somehow ran out of virtual memory space. Maybe
    // something is blowing up the size of the kernel?
    KERNEL_ASSERT(page_count > USER_RO_PAGES);

    u32 kernel_page_count = page_count - USER_RO_PAGES;
    void *user_ro_addr = heap_addr + kernel_page_count * PAGE_SIZE;

    u16 kernel_flags = PAGE_WRITABLE | PAGE_GLOBAL;
    u16 user_flags = PAGE_WRITABLE | PAGE_USER_MODE | PAGE_GLOBAL;

    heap_init(&kernel_heap, heap_addr, kernel_page_count, kernel_flags);
    heap_init(&user_ro_heap, user_ro_addr, USER_RO_PAGES, user_flags);
}

// Allocates a given number of pages on the heap. Empty allocations are not
// allowed.
void *mem_alloc(Heap *heap, u32 pages, u16 flags) {
    KERNEL_ASSERT(pages);
    void *vaddr = heap_alloc(heap, pages);
    alloc_pages(vaddr, flags, pages);
    return vaddr;
}

// Reallocates an existing allocation to a new size. This may require moving the
// pointer and copying the data so the new pointer is returned. If a different
// pointer is returned, the old pointer is invalidated.
void *mem_realloc(Heap *heap, void *ptr, u32 pages) {
    KERNEL_ASSERT(pages);

    void *new_ptr;
    u32 old_size = heap_realloc(heap, ptr, &new_ptr, pages);

    // Permissions should be the same for all pages in a single allocation.
    u16 flags = get_flags(get_entry(ptr));

    if (new_ptr != ptr) {
        alloc_pages(new_ptr, flags, pages);
        pmemcpy(new_ptr, ptr, old_size * PAGE_SIZE);
        free_pages(ptr, old_size);
    }
    else {
        if (old_size > pages) {
            void *new_alloc_end = ptr + pages * PAGE_SIZE;
            free_pages(new_alloc_end, old_size - pages);
        }
        else {
            void *old_alloc_end = ptr + old_size * PAGE_SIZE;
            alloc_pages(old_alloc_end, flags, pages - old_size);
        }
    }

    return new_ptr;
}

// Frees an existing allocation.
void mem_free(Heap *heap, void *ptr) {
    u32 freed_count = heap_free(heap, ptr);
    free_pages(ptr, freed_count);
}

// Kernel heap allocator

void *kernel_alloc(u32 pages) {
    return mem_alloc(&kernel_heap, pages, PAGE_WRITABLE);
}

void *kernel_realloc(void *ptr, u32 pages) {
    return mem_realloc(&kernel_heap, ptr, pages);
}

void kernel_free(void *ptr) {
    return mem_free(&kernel_heap, ptr);
}

// Ensures that heap pages are marked as used if and only if their corresponding
// page table entry is present. This function checks every page on the heap and
// thus should only be used for debugging purposes. We also print a
// representation of the usage of the first `pages` pages of the heap.
void debug_heap(u32 pages) {
    void *end = (void *) page_table_entries;
    for (void *addr = kernel_heap.heap_start; addr < end; addr += PAGE_SIZE) {
        KERNEL_ASSERT(
            heap_is_used(&kernel_heap, addr) == entry_present(get_entry(addr))
        );
    }

    heap_debug(&kernel_heap, pages);
}

// Creates a new page directory and returns its frame address
u32 new_page_dir() {
    u32 *dir = kernel_alloc(1);

    for (u32 i = 1; i < KERNEL_PDI_START; ++i)
        dir[i] = 0; // the present flag gets cleared

    for (u32 i = KERNEL_PDI_START; i < KERNEL_PDI_END; ++i)
        dir[i] = PDE(i); // copy the kernel pages

    // Lower memory map
    dir[get_pd_index(0)] = PDE(0);
    // Page directory self-map. This needs to be writable or we won't be able to
    // edit our page structures.
    dir[1023] = create_entry(get_paddr(get_entry(dir)), PDE_WRITABLE);

    // We don't actually want to free the frame
    heap_free(&kernel_heap, dir);
    u32 entry;
    KERNEL_ASSERT(!unmap_page(dir, &entry));
    return get_paddr(entry);
}

// Checks if a given pointer is in userspace.
void *validate_user_readable(u32 vaddr) {
    u32 entry = get_entry((void *) vaddr);
    u32 present = entry_present(entry);
    u32 user_mode = entry & PTE_USER_MODE;

    if (present && user_mode)
        return (void *) vaddr;
    else
        return NULL;
}

// Checks if a given pointer is writeable in userspace.
void *validate_user_writable(u32 vaddr) {
    u32 entry = get_entry((void *) vaddr);
    u32 present = entry_present(entry);
    u32 user_mode = entry & PTE_USER_MODE;
    u32 writable = entry & PTE_WRITABLE;

    if (present && user_mode && writable)
        return (void *) vaddr;
    else
        return NULL;
}

// Checks if a given frame is reserved for user programs.
bool user_frame_valid(u32 paddr) {
    for (u32 i = 0; i < user_physical_map_len; ++i) {
        u32 start_addr = user_physical_map[i].addr;
        u32 end_addr = user_physical_map[i].addr + user_physical_map[i].len;

        if (start_addr <= paddr && paddr < end_addr)
            return true;
    }

    return false;
}

#define GET_PHYS_MAP_INVALID_PTR 1

// Returns a pointer to information about the physically available memory on the
// system. Writes the number of entries to `n_ptr`. Each entry contains a
// physical address and size in bytes.
SyscallResult syscall_get_phys_map(u32 n_ptr) {
    u32 *n = validate_user_writable(n_ptr);
    if (!n)
        SYSCALL_RETURN(0, GET_PHYS_MAP_INVALID_PTR);

    *n = user_physical_map_len;

    SYSCALL_RETURN((u32) user_physical_map, 0);
}

#define VIRT_MAP_PID_NOT_FOUND  1
#define VIRT_MAP_ALREADY_MAPPED 2
#define VIRT_MAP_INVALID_PTR    3
#define VIRT_MAP_INVALID_PADDR  4

// Converts syscall API flags to page table flags
static u16 get_pte_flags(u16 flags) {
    u16 pte_flags = PTE_USER_MODE;
    if (flags & VIRT_MAP_WRITABLE)
        pte_flags |= PTE_WRITABLE;
    return pte_flags;
}

// Maps a contiguous region of `n` pages starting at `vaddr` in the address
// space specified by `aid` using the physical addresses given in `paddr_ptr`,
// an array of size `n`. `vaddr` will actually start at its page if its not
// already aligned on a page boundary.
SyscallResult
syscall_virt_map(u32 aid, u32 vaddr, u32 paddr_ptr, u32 n, u32 flags) {
    Process *process = NULL;
    if (aid >> 16 == 0) // aid must be 16 bit
        process = get_process(aid);
    if (!process)
        SYSCALL_RETURN(0, VIRT_MAP_PID_NOT_FOUND);

    u32 *paddr = validate_user_readable(paddr_ptr);
    if (!paddr)
        SYSCALL_RETURN(0, VIRT_MAP_INVALID_PTR);

    u32 pte_flags = get_pte_flags(flags);
    u32 old_pd_paddr = get_page_dir();
    set_page_dir(process->page_dir_paddr);

    for (u32 i = 0; i < n; ++i) {
        void *page_vaddr = (void *) vaddr + i * PAGE_SIZE;
        if (entry_present(get_entry(page_vaddr))) {
            set_page_dir(old_pd_paddr);
            SYSCALL_RETURN(0, VIRT_MAP_ALREADY_MAPPED);
        }
        if (!user_frame_valid(paddr[i])) {
            set_page_dir(old_pd_paddr);
            SYSCALL_RETURN(0, VIRT_MAP_INVALID_PADDR);
        }
    }

    for (u32 i = 0; i < n; ++i) {
        void *page_vaddr = (void *) vaddr + i * PAGE_SIZE;
        KERNEL_ASSERT(!map_page(page_vaddr, paddr[i], pte_flags));
    }

    set_page_dir(old_pd_paddr);
    SYSCALL_RETURN(0, 0);
}

#define VIRT_UNMAP_PID_NOT_FOUND 1
#define VIRT_UNMAP_NOT_MAPPED    2
#define VIRT_UNMAP_INVALID_PTR   3

// Unmaps a region of `n` contiguous pages in the address space specified by
// `aid` starding at `vaddr`. Returns the backing physical pages into the array
// of size `n` specified by `paddr_ptr`.
SyscallResult syscall_virt_unmap(u32 aid, u32 vaddr, u32 paddr_ptr, u32 n) {
    Process *process = NULL;
    if (aid >> 16 == 0) // aid must be 16 bit
        process = get_process(aid);
    if (!process)
        SYSCALL_RETURN(0, VIRT_UNMAP_PID_NOT_FOUND);

    u32 *paddr = validate_user_writable(paddr_ptr);
    if (!paddr)
        SYSCALL_RETURN(0, VIRT_UNMAP_INVALID_PTR);

    u32 old_pd_paddr = get_page_dir();
    set_page_dir(process->page_dir_paddr);

    for (u32 i = 0; i < n; ++i) {
        void *page_vaddr = (void *) vaddr + i * PAGE_SIZE;
        if (!entry_present(get_entry(page_vaddr))) {
            set_page_dir(old_pd_paddr);
            SYSCALL_RETURN(0, VIRT_UNMAP_NOT_MAPPED);
        }
    }

    for (u32 i = 0; i < n; ++i) {
        u32 entry;
        void *page_vaddr = (void *) vaddr + i * PAGE_SIZE;
        KERNEL_ASSERT(!unmap_page(page_vaddr, &entry));
        paddr[i] = get_paddr(entry);
    }

    set_page_dir(old_pd_paddr);
    SYSCALL_RETURN(0, 0);
}

#define VIRT_TRANSFER_PID_INVALID   1
#define VIRT_TRANSFER_VADDR_INVALID 2
#define VIRT_TRANSFER_TOO_LARGE     3

SyscallResult
syscall_virt_transfer(void *vaddr_src, void *vaddr_dst, u32 aid_pair, u32 n) {
    u32 *entries = (u32 *) temp_buffer;
    u32 entries_len = TEMP_BUFFER_LENGTH / sizeof(u32);

    // NOTE: This will limit the number of pages you can transfer at once. At
    // the time of writing, the temp buffer is 8KiB long so we are limited to
    // 2048 pages transferred at a time. This should be enough for now. If/when
    // it becomes an issue, we can allocate space on the kernel heap or
    // something to do larger transfers.
    if (n > entries_len)
        SYSCALL_RETURN(0, VIRT_TRANSFER_TOO_LARGE);

    u32 aid_src = aid_pair & 0xFFFF;
    u32 aid_dst = (aid_pair >> 16) & 0xFFFF;

    Process *proc_src = get_process(aid_src);
    Process *proc_dst = get_process(aid_dst);

    if (!proc_src || !proc_dst)
        SYSCALL_RETURN(0, VIRT_TRANSFER_PID_INVALID);

    u32 old_pd_paddr = get_page_dir();
    set_page_dir(proc_src->page_dir_paddr);

    for (u32 i = 0; i < n; ++i) {
        void *page_vaddr = (void *) vaddr_src + i * PAGE_SIZE;
        if (!entry_present(get_entry(page_vaddr))) {
            set_page_dir(old_pd_paddr);
            SYSCALL_RETURN(0, VIRT_TRANSFER_VADDR_INVALID);
        }
    }

    for (u32 i = 0; i < n; ++i) {
        void *page_vaddr = (void *) vaddr_src + i * PAGE_SIZE;
        KERNEL_ASSERT(!unmap_page(page_vaddr, entries + i));
    }

    set_page_dir(proc_dst->page_dir_paddr);

    void *vaddr_final = vaddr_dst;
    u32 error = 0;

    for (u32 i = 0; i < n; ++i) {
        void *page_vaddr = (void *) vaddr_dst + i * PAGE_SIZE;
        if (entry_present(get_entry(page_vaddr))) {
            error = VIRT_TRANSFER_VADDR_INVALID;
            break;
        }
    }

    if (error) {
        // If the mapping fails, we need to remap into the source address space
        // to undo our partial progress.
        vaddr_final = vaddr_src;
        set_page_dir(proc_src->page_dir_paddr);
    }

    for (u32 i = 0; i < n; ++i) {
        void *page_vaddr = (void *) vaddr_final + i * PAGE_SIZE;
        u32 paddr = get_paddr(entries[i]);
        u16 flags = get_flags(entries[i]);

        KERNEL_ASSERT(!map_page(page_vaddr, paddr, flags));
    }

    set_page_dir(old_pd_paddr);
    SYSCALL_RETURN(0, error);
}

// Performs all operations required to initialize our kernel memory management.
void mem_init() {
    current_page_dir = get_cr3();
    kernel_page_dir = get_page_dir();
    init_free_list_page();
    init_frames();
    reinit_paging();
    init_heap();

    // FIXME: our use of the PAGE_WRITEABLE flag is very questionable. Here we
    // rely on the kernel having write access to this memory despite not setting
    // the flag. But in other places we set the flag when we "need" write
    // access. We probably should be consistent.
    user_physical_map = mem_alloc(&user_ro_heap, 1, PAGE_USER_MODE);

    pmemcpy(
        user_physical_map, user_physical_map_buffer,
        user_physical_map_len * sizeof(PhysicalMap)
    );

    register_syscall(6, syscall_get_phys_map);
    register_syscall(7, syscall_virt_map);
    register_syscall(8, syscall_virt_unmap);
    register_syscall(9, syscall_virt_transfer);
}
