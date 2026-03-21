#ifndef SUN_H_
#define SUN_H_

#include "types.h"

// Kernel assertion will fail if more than this number of pages is present
#define MAX_SUNFILE_PAGES 8

typedef struct {
    char name[16];
    u32 offset;
    void (*entry_point)();
    u32 text_size;
    u32 rodata_size;
    u32 data_size;
    u32 bss_size;
} TableEntry;

typedef struct {
    char magic[3];
    u8 n;
    TableEntry entries[];
} SunFile;

_Static_assert(sizeof(TableEntry) == 40, "Table entry size mismatch");

void sun_init();
TableEntry *sun_exe_lookup(const char *name);
void sun_load_text(TableEntry *entry, u8 *buffer);
void sun_load_rodata(TableEntry *entry, u8 *buffer);
void sun_load_data(TableEntry *entry, u8 *buffer);
u32 sunfile_size();

#endif // SUN_H_
