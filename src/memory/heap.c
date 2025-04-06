#include "heap.h"
#include "drivers/serial/io.h"
#include "lib/types.h"
#include "memory/mem.h"
#include "lib/error.h"
#include "lib/libp.h"
#include "terminal/terminal.h"

#define HEAP_PAGE_FREE       0
#define HEAP_PAGE_USED_RED   1
#define HEAP_PAGE_USED_GREEN 2
#define HEAP_PAGE_USED_BLUE  3

// NOTE: all of these functions that search by bits can be greatly optimized by
// searching by bytes or even dwords instead.

bool is_page_aligned(void *ptr) {
    return ((u32) ptr << 20) == 0;
}

// Returns the two bits representing the usage state of a given page. The state
// can be FREE, USED_RED, USED_GREEN, or USED_BLUE. `index` should be the index
// of the page we are checking starting from the first page of the heap.
u8 heap_get_usage(Heap *heap, u32 index) {
    u32 byte_idx = index / 4;
    u32 bit_pair_idx = 6 - 2 * (index % 4);

    return (heap->usage_map[byte_idx] >> bit_pair_idx) & 0b11;
}

// Sets the usage state of a given page. See `heap_get_usage`
void heap_set_usage(Heap *heap, u32 index, u8 usage) {
    u32 byte_idx = index / 4;
    u32 bit_pair_idx = 6 - 2 * (index % 4);

    heap->usage_map[byte_idx] =
        (heap->usage_map[byte_idx] & ~((u8)0b11 << bit_pair_idx)) |
        (usage << bit_pair_idx);
}

// Check if a given amount of space exists at a particular location. `start` is
// the page index from the first page of the heap and `space` is the required
// space in pages.
bool heap_is_space_free(Heap *heap, u32 start, u32 space) {
    for (unsigned i = start; space--; ++i) {
        if (heap_get_usage(heap, i) != HEAP_PAGE_FREE)
            return FALSE;
    }

    return TRUE;
}

// Returns a valid USED state color based on the states of the pages bordering
// the region. The requirement is that the color cannot be equal to left or
// right and must be one of the USED states.
u8 get_new_color(u8 left, u8 right) {
    u8 color = left;
    for (u8 i = 0; i < 3; ++i) {
        color += 1;
        if (color > 3) color = 1;
        if (color != left && color != right)
            return color;
    }
    KERNEL_ASSERT(FALSE);
}

// Marks a region of the heap as used. `start` should be the page index starting
// from the first heap page and `space` is the number of pages from there to
// mark as used. Any subregion inside the specified region can also be used but
// all subregions will be merged into one.
void heap_mark_used(Heap *heap, u32 start, u32 space) {
    u32 end = start + space - 1;

    u8 left = HEAP_PAGE_FREE;
    u8 right = HEAP_PAGE_FREE;
    if (start > 0)
        left = heap_get_usage(heap, start - 1);
    if (end < heap->page_count - 1)
        right = heap_get_usage(heap, end + 1);

    u8 color = get_new_color(left, right);

    for (u32 i = 0; i < space; ++i) {
        heap_set_usage(heap, start + i, color);
    }
}

void heap_init(Heap *heap, void *heap_start, u32 page_count) {
    KERNEL_ASSERT(is_page_aligned(heap_start));

    // size of our usage map
    u32 byte_size = (page_count + 3) / 4;
    u32 page_size = (byte_size + PAGE_SIZE - 1) / PAGE_SIZE;

    alloc_pages(heap_start, page_size);
    pmemset(heap_start, HEAP_PAGE_FREE, byte_size);

    heap->usage_map = heap_start;
    heap->heap_start = heap_start + page_size * PAGE_SIZE; // account for usage map
    heap->page_count = page_count;
}

// Finds a large enough space on the heap to allocate some memory. Uses first
// fit.
void *heap_alloc(Heap *heap, u32 pages) {
    u32 space = 0;
    u32 start = 0;

    for (u32 i = 0; i < heap->page_count; ++i) {
        u8 usage = heap_get_usage(heap, i);

        if (usage == HEAP_PAGE_FREE) {
            space += 1;

            if (space >= pages) {
                heap_mark_used(heap, start, space);
                return heap->heap_start + start * PAGE_SIZE;
            }
        }
        else {
            space = 0;
            start = i + 1;
        }
    }

    // NOTE: For now we will just panic if we run out of address space. Really
    // we should propagate some kind of error upwards, maybe by returning a null
    // pointer, since running out of memory is not a fatal error. We can
    // mitigate this case with a better page allocation algorithm.

    KERNEL_ASSERT(FALSE); // Out of memory
}

// Reallocates some memory. Returns the size of the old allocation.
u32 heap_realloc(Heap *heap, void *ptr, void **new_ptr, u32 pages) {
    KERNEL_ASSERT(is_page_aligned(ptr)); // We only give out page-aligned pointers

    u32 start = (ptr - heap->heap_start) / PAGE_SIZE;
    u32 color = heap_get_usage(heap, start);
    u32 end = start + 1;
    u32 realloc_end = start + pages;

    KERNEL_ASSERT(color != HEAP_PAGE_FREE);

    while (heap_get_usage(heap, end) == color) end += 1;

    if (realloc_end <= end) { // downsizing?
        for (u32 i = realloc_end; i < end; ++i)
            heap_set_usage(heap, i, HEAP_PAGE_FREE);
        *new_ptr = ptr;
        return end - start;
    }

    if (heap_is_space_free(heap, end, realloc_end - end)) {
        // there's enough space to expand
        if (heap_get_usage(heap, realloc_end) == color) {
            // we ran into a same-colored allocation and now need to re-color
            heap_mark_used(heap, start, pages);
        }
        else {
            // we can use the current allocation color
            for (u32 i = end; i < realloc_end; ++i)
                heap_set_usage(heap, i, color);
        }

        *new_ptr = ptr;
        return end - start;
    } else {
        *new_ptr = heap_alloc(heap, pages);
        return heap_free(heap, ptr);
    }
}

u32 heap_free(Heap *heap, void *ptr) {
    KERNEL_ASSERT(is_page_aligned(ptr)); // We only give out page-aligned pointers

    u32 start = (ptr - heap->heap_start) / PAGE_SIZE;
    u32 color = heap_get_usage(heap, start);

    KERNEL_ASSERT(color != HEAP_PAGE_FREE);

    u32 freed_count = 0;
    for (u32 i = start; heap_get_usage(heap, i) == color; ++i) {
        freed_count += 1;
        heap_set_usage(heap, i, HEAP_PAGE_FREE);
    }

    return freed_count;
}

// The following functions are only used for debugging.

// Prings a representation of the first `pages` pages of `heap`.
void heap_debug(Heap *heap, u32 pages) {
    if (pages > heap->page_count)
        pages = heap->page_count;

    const VgaColor colors[4] = { VGA_COLOR_WHITE, VGA_COLOR_RED, VGA_COLOR_GREEN, VGA_COLOR_BLUE };
    const char *chars = ".RGB";

    for (u32 i = 0; i < pages; ++i) {
        u8 usage = heap_get_usage(heap, i);
        terminal.color = vga_color_create(VGA_COLOR_BLACK, colors[usage]);
        terminal_putchar(' ');
        serial_write(chars[usage]);
    }

    terminal.color = vga_color_create(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_putchar('\n');
}

// Determines if a given address on `heap` is currently in use.
bool heap_is_used(Heap *heap, void *addr) {
    u32 i = (addr - heap->heap_start) / PAGE_SIZE;
    return heap_get_usage(heap, i) != HEAP_PAGE_FREE;
}
