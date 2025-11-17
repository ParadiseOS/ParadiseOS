#include "util.h"

u16 get_pid_aid(u32 pid) {
    return pid & 0xFFFF;
}

u16 get_pid_tid(u32 pid) {
    return (pid >> 16) & 0xFFFF;
}