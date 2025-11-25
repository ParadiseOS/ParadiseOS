#include "util.h"

u16 get_pid_aid(u32 pid) {
    return pid >> 16;
}

u16 get_pid_tid(u32 pid) {
    return pid & 0xFFFF;
}

char temp_buffer[TEMP_BUFFER_LENGTH] __attribute__((aligned(4)));
