#ifndef POOL_H_
#define POOL_H_

#include "mem.h"
#include "processes.h"
#include "types.h"

typedef struct {
    Process *free_list;
} ProcessPool;

_Static_assert(sizeof(Process) <= PAGE_SIZE, "Process struct too big");

// Right now the implementation will never decrease its capacity. Memory will
// still be reused as processes are destroyed.

void pool_init(ProcessPool *pool);
Process *pool_create(ProcessPool *pool);
void pool_destroy(ProcessPool *pool, Process *proc);

#endif // POOL_H_
