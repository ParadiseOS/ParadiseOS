#include <paradise/logging.h>
#include <paradise/error.h>
#include <paradise/libp.h>
#include <paradise/sun.h>
#include <paradise/mem.h>


extern SunFile sun_file;

static char MAGIC[] = "SUN";
static u32 MAGIC_LEN = sizeof(MAGIC) - 1;

void sun_init() {
    KERNEL_ASSERT(pmemeql(sun_file.magic, MAGIC, MAGIC_LEN));

    //Assert the file ins't too large
    KERNEL_ASSERT((sizeof(SunFile) + sizeof(TableEntry)) / PAGE_SIZE < MAX_SUNFILE_PAGES);
}

TableEntry *sun_exe_lookup(const char *name) {
    for (u8 i = 0; i < sun_file.n; ++i) {
        if (pstreql(name, sun_file.entries[i].name))
            return sun_file.entries + i;
    }

    return NULL;
}

u32 sunfile_size() {
    TableEntry *last_entry = &sun_file.entries[sun_file.n - 1];
    
    u32 entry_data_size = last_entry->text_size + 
                          last_entry->rodata_size + 
                          last_entry->data_size + 
                          last_entry->bss_size;

    u32 offset = last_entry -> offset;
    u32 size = offset + entry_data_size;
    
    return size;
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
