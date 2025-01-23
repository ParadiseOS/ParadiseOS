#ifndef MULTIBOOT_H_
#define MULTIBOOT_H_

// See https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#Boot-information-format

#include "lib/types.h"

#define MMAP_AVAILABLE 1
#define MMAP_RESERVED  2
#define MMAP_ACPI      3
#define MMAP_NVS       4
#define MMAP_DEFECTIVE 5

#define MB_FLAG_MMAP        (1 << 6)
#define MB_FLAG_FRAMEBUFFER (1 << 12)

typedef struct __attribute__((__packed__)) {
    u32 size;
    u32 base_addr_lo;
    u32 base_addr_hi;
    u32 length_lo;
    u32 length_hi;
    u32 type;
} MMapEntry;

typedef struct __attribute__((packed)) {
    u32 flags;

    u32 mem_lower;
    u32 mem_upper;

    u32 boot_device;

    u32 cmdline;

    u32 mods_count;
    void *mods_addr;

    u32 syms[4];

    u32 mmap_length;
    MMapEntry *mmap_addr;

    u32 unused[9];

    u32 framebuffer_addr_lo;
    u32 framebuffer_addr_hi;
    u32 framebuffer_pitch;
    u32 framebuffer_width;
    u32 framebuffer_height;
} MultibootInfo;

#endif // MULTIBOOT_H_
