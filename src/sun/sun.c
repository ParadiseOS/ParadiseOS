#include "sun.h"
#include "lib/error.h"
#include "lib/libp.h"

typedef struct {
    char magic[3];
    u8 n;
    TableEntry entries[];
} SunFile;

extern SunFile sun_file;

static char MAGIC[] = "SUN";
static u32 MAGIC_LEN = sizeof(MAGIC) - 1;

void sun_init() {
    KERNEL_ASSERT(pmemeql(sun_file.magic, MAGIC, MAGIC_LEN));
}

TableEntry *sun_exe_lookup(const char *name) {
    for (u8 i = 0; i < sun_file.n; ++i) {
        if (pstreql(name, sun_file.entries[i].name))
            return sun_file.entries + i;
    }

    return NULL;
}

void sun_load_text(TableEntry *entry, u8 *buffer) {
    pmemcpy(buffer, ((char *) &sun_file) + entry->offset, entry->text_size);
}

void sun_load_rodata(TableEntry *entry, u8 *buffer) {
    u32 offset = entry->offset + entry->text_size;
    pmemcpy(buffer, ((char *) &sun_file) + offset, entry->rodata_size);
}

void sun_load_data(TableEntry *entry, u8 *buffer) {
    u32 offset = entry->offset + entry->text_size + entry->rodata_size;
    pmemcpy(buffer, ((char *) &sun_file) + offset, entry->data_size);
}
