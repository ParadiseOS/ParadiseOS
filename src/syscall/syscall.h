#ifndef SYSCALL_H_
#define SYSCALL_H_

#include "lib/types.h"
#include "process/processes.h"

typedef struct {
    u32 ret;
    u32 err;
} SyscallResult;

#define SYSCALL_RET(ret)                                                       \
    return (SyscallResult) { ret, 0 }

#define SYSCALL_ERR(err)                                                       \
    return (SyscallResult) { 0, err }

void register_syscall(u32 num, void *syscall);
bool dispatch_syscall(CpuContext *ctx);
void *delete_syscall(u32 num);
void syscalls_init();

#endif // SYSCALL_H_
