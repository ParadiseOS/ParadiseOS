#ifndef PSYSCALL_H_
#define PSYSCALL_H_

#include "types.h"

// List of Syscalls Numbers (for convience)

#define SYSCALL_SEND_MESSAGE 0
#define SYSCALL_READ_MESSAGE 1
#define SYSCALL_REG_PROC     2
#define SYSCALL_DEL_PROC     3
#define SYSCALL_JUMP_PROC    4
#define SYSCALL_REG_TMR_CB   5
#define SYSCALL_GET_PHYS_MAP 6
#define SYSCALL_VIRT_MAP     7
#define SYSCALL_VIRT_UNMAP   8
#define SYSCALL_VIRT_TRANS   9
#define SYSCALL_PUTSN        99
#define SYSCALL_PUTS         100

typedef struct {
    u32 ret;
    u32 err;
} SyscallReturn;

SyscallReturn psyscall(u32 num, u32 a, u32 b, u32 c, u32 d, u32 e);

#endif // PSYSCALL_H_
