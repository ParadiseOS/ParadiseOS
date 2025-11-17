#include "syscall.h"
#include "lib/error.h"
#include "process/processes.h"

#define NUM_SYSCALLS 101

void *syscall_table[NUM_SYSCALLS] = {NULL};

typedef SyscallResult (*Syscall)(u32 a, u32 b, u32 c, u32 d, u32 e);

void register_syscall(u32 num, void *syscall) {
    KERNEL_ASSERT(num < NUM_SYSCALLS);
    KERNEL_ASSERT(!syscall_table[num]);
    syscall_table[num] = syscall;
}

// The PCB must be updated with the processes current registers. It will be the
// input and output of this function. Returns true if the syscall exists.
bool dispatch_syscall(CpuContext *ctx) {
    if (ctx->eax >= NUM_SYSCALLS)
        return false;
    Syscall syscall = syscall_table[ctx->eax];
    if (!syscall)
        return false;

    // This works regardless of the number of actual expected parameters since
    // the callee is responsible for cleaning up the stack.
    SyscallResult res =
        syscall(ctx->ebx, ctx->ecx, ctx->edx, ctx->esi, ctx->edi);
    ctx->eax = res.ret;
    ctx->ebx = res.err;
    return true;
}

void *delete_syscall(u32 num) {
    KERNEL_ASSERT(num < NUM_SYSCALLS);
    void *syscall = syscall_table[num];
    KERNEL_ASSERT(syscall);
    syscall_table[num] = NULL;
    return syscall;
}
