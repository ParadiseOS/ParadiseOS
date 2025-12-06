#ifndef HEAP_H_
#define HEAP_H_

#include "types.h"

// A very simple page allocator implemented with a bit vector representing heap
// usage. Two bits are assigned per page.
//
//     0b00 -> Free
//     0b01 -> Used Red
//     0b10 -> Used Blue
//     0b11 -> Used Green
//
// There are different page colors so that we can keep track of the extent of
// different allocation so that the correct number of pages can be freed. Three
// different colors are used to guarantee that there is never a conflict. At
// worst, allocation on either side of a given allocation are different colors,
// so at most a third color is needed to discern between the allocations.
// Technically, a 1D map is two-colorable but may require recoloring in our
// case. Using three colors saves us from that.
//
typedef struct {
    u8 *usage_map;
    void *heap_start;
    u32 page_count;
} Heap;

void heap_init(Heap *heap, void *heap_start, u32 page_count, u16 flags);

void *heap_alloc(Heap *heap, u32 pages);
u32 heap_realloc(Heap *heap, void *ptr, void **new_ptr, u32 pages);
u32 heap_free(Heap *heap, void *ptr);

void heap_debug(Heap *heap, u32 pages);
bool heap_is_used(Heap *heap, void *addr);

#endif // HEAP_H_
