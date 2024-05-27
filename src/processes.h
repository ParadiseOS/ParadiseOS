#ifndef PROCESS_H_
#define PROCESS_H_

#include "types.h"

typedef struct __attribute__((packed, aligned(4096))) {
    u16 prev_task_link;
    u16 reserved1;
    u32 esp0;
    u16 ss0;
    u16 reserved2;
    u32 esp1;
    u16 ss1;
    u16 reserved3;
    u32 esp2;
    u16 ss2;
    u16 reserved4;
    u32 page_directory;
    u32 eip;
    u32 EFLAGS;
    u32 eax;
    u32 ecx;
    u32 edx;
    u32 ebx;
    u32 esp;
    u32 ebp;
    u32 esi;
    u32 edi;
    u16 ES;
    u16 reserved5;
    u16 CS;
    u16 reserved6;
    u16 SS;
    u16 reserved7;
    u16 DS;
    u16 reserved8;
    u16 FS;
    u16 reserved9;
    u16 GS;
    u16 reserved10;
    u16 LDT;
    u16 reserved11;
    u16 trap;
    u16 io_base;
} Tss;

extern u32 load_tss(Tss *);

#endif
