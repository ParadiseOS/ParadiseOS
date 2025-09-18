#include "syscall.h"
#include "lib/error.h"

#define NUM_SYSCALLS 16

void *syscall_table[NUM_SYSCALLS] = { NULL };

typedef SyscallResult (*Syscall)(u32 a, u32 b, u32 c, u32 d, u32 e);

void register_syscall(u32 num, void *syscall) {
    KERNEL_ASSERT(num < NUM_SYSCALLS);
    KERNEL_ASSERT(!syscall_table[num]);
    syscall_table[num] = syscall;
}

// The PCB must be updated with the processes current registers. It will be the
// input and output of this function.
void dispatch_syscall(u32 num, ProcessControlBlock *pcb) {
    KERNEL_ASSERT(num < NUM_SYSCALLS);
    Syscall syscall = syscall_table[num];
    KERNEL_ASSERT(syscall);
    // This works regardless of the number of actual expected parameters since
    // the callee is responsible for cleaning up the stack.
    SyscallResult res = syscall(pcb->ebx, pcb->ecx, pcb->edx, pcb->esi, pcb->edi);
    pcb->eax = res.ret;
    pcb->ebx = res.err;
}

void *delete_syscall(u32 num) {
    KERNEL_ASSERT(num < NUM_SYSCALLS);
    void *syscall = syscall_table[num];
    KERNEL_ASSERT(syscall);
    syscall_table[num] = NULL;
    return syscall;
}
