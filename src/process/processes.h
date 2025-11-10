#ifndef PROCESS_H_
#define PROCESS_H_

#include "lib/types.h"
#include "memory/heap.h"
#include "memory/mem.h"
#include "rb_tree.h"
#include "queue.h"

typedef struct {
    u32 page_dir_paddr;

    // Since these nodes are stored at a known offset in Process, we can always
    // determine the process corresponding to a given node.
    QueueNode queue_node;
    RbNode rb_node;

    bool blocked;
} Process;

// NOTE: we assume the offsets of members up to eflags in the assembly so things
// will break if thats changed
typedef struct {
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    u32 eflags;
    u32 eip;

    void *prog_brk;
    u32 page_dir_paddr;

    u8 fpu_regs[512] __attribute__((aligned(16)));

    Heap heap;
} ProcessControlBlock;

_Static_assert(sizeof(ProcessControlBlock) <= PAGE_SIZE, "PCB too large");

typedef struct {
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    u32 eip, cs, eflags, useresp;
} CpuContext;

#define GET_PID(proc) (proc->rb_node.key)

extern Process *current;
extern CpuContext *current_ctx;

void exec_sun(const char *name, int arg);
__attribute__((noreturn)) void schedule();
void processes_init();
Process *get_process(u32 pid);

#endif
