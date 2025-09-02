#include "pool.h"
#include "lib/error.h"
#include "memory/mem.h"

void pool_init(ProcessPool *pool) {
    pool->free_list = NULL;
}

Process *pool_create(ProcessPool *pool) {
    if (!pool->free_list) {
        Process *processes = kernel_alloc(1);
        u32 n = PAGE_SIZE / sizeof(Process);

        while (n--)
            pool_destroy(pool, processes + n);
    }

    Process *proc = pool->free_list;
    pool->free_list = *((Process **) proc);
    return proc;
}

void pool_destroy(ProcessPool *pool, Process *proc) {
    *((Process **) proc) = pool->free_list;
    pool->free_list = proc;
}
