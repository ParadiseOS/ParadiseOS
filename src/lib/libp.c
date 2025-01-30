#include "libp.h"

void pmemset(void *memory, u8 byte, u32 count) {
    char *bytes = memory;

    for (u32 i = 0; i < count; ++i) {
        bytes[i] = byte;
    }
}
