#include <paradise/allocator_test.h>
#include <paradise/error.h>
#include <paradise/heap.h>
#include <paradise/libp.h>
#include <paradise/mem.h>
#include <paradise/terminal.h>
#include <paradise/types.h>

typedef struct Allocation_ Allocation;

struct Allocation_ {
    u32 size;
    void *ptr;
    Allocation *next;
};

u32 allocated = 0;

Allocation *allocate(Allocation *head, u32 pages) {
    Allocation *a = kernel_alloc(1);
    allocated += pages;
    a->size = pages;
    a->ptr = kernel_alloc(pages);
    a->next = head;
    return a;
}

Allocation *free_i(Allocation *head, u32 i) {
    Allocation *l = NULL;
    Allocation *r = head;

    while (i--) {
        l = r;
        r = r->next;
    }

    allocated -= r->size;
    kernel_free(r->ptr);

    Allocation *new_head;
    if (l) {
        l->next = r->next;
        new_head = head;
    }
    else {
        new_head = r->next;
    }

    kernel_free(r);
    return new_head;
}

void realloc_i(Allocation *head, u32 pages, u32 i) {
    Allocation *a = head;
    while (i--)
        a = a->next;

    allocated += pages;
    allocated -= a->size;
    a->ptr = kernel_realloc(a->ptr, pages);
}

void debug(Allocation *head) {
    if (head == NULL) {
        terminal_printf("EMPTY\n");
        return;
    }

    const char *delim = "";

    for (Allocation *a = head; a; a = a->next) {
        terminal_printf("%s%u", delim, a->size);
        delim = "->";
    }

    terminal_printf("\n");
}

const u32 N = 2000;

void test_allocator() {
    Prng rng;
    prng_init(&rng, 3);

    Allocation *head = NULL;
    u32 count = 0;

    for (u32 i = 0; i < N; ++i) {
        u32 choice = head ? prng_next(&rng) % 6 : 5;
        if (choice <= 0) {
            head = free_i(head, prng_next(&rng) % count);
            --count;
        }
        else if (choice <= 2) {
            realloc_i(
                head, (prng_next(&rng) & 0xFF) + 1, prng_next(&rng) % count
            );
        }
        else {
            head = allocate(head, (prng_next(&rng) & 0xFF) + 1);
            ++count;
        }

        if (allocated >= 0x4000) {
            break;
        }
    }

    debug_heap(allocated);
}
