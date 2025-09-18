#ifndef SYSCALL_H_
#define SYSCALL_H_

#include "lib/types.h"
#include "process/processes.h"

typedef struct {
    u32 ret;
    u32 err;
} SyscallResult;

#define SYSCALL_RETURN(ret, err)                                               \
    do {                                                                       \
        return (SyscallResult) { ret, err };                                   \
    } while (false)

void register_syscall(u32 num, void *syscall);
void dispatch_syscall(u32 num, ProcessControlBlock *pcb);
void *delete_syscall(u32 num);

#endif // SYSCALL_H_
