#include <psyscall.h>

SyscallReturn psyscall(u32 num, u32 a, u32 b, u32 c, u32 d, u32 e) {
    SyscallReturn ret;

    asm volatile (
        "int 0x80"
        : "=a"(ret.ret), "=b"(ret.err)             // Outputs
        : "a"(num), "b"(a), "c"(b), "d"(c), "S"(d), "D"(e) // Inputs
        : "memory"                                 // Clobbers
    );

    return ret;
}
