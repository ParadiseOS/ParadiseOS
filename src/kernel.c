#include "terminal.h"
#include "multiboot.h"
#include "init.h"
#include "error.h"
#include "mem.h"
#include "processes.h"

extern const u32 *multiboot_info;
extern const u32 *stack_top;
extern const u32 *_start;

extern DescriptorEntry gdt[3];
extern DescriptorEntry idt[256];
extern Tss initial_tss;


void kernel_main(void) {
    MultibootInfo *mb_info = (MultibootInfo *) multiboot_info;
    u32 mb_flags = mb_info->flags;

    if (!(mb_info->flags & MB_FLAG_FRAMEBUFFER)) {
        // No terminal info is available. Something went wrong and there's no
        // way to report it.
        kernel_panic();
    }

    terminal_init(
        mb_info->framebuffer_width,
        mb_info->framebuffer_height,
        (u16 *) mb_info->framebuffer_addr_lo
    );

    KERNEL_ASSERT(mb_info->framebuffer_addr_hi == 0);


    terminal_printf("Hello, Paradise!\n");

    terminal_printf("Multiboot flags: %b\n", mb_flags);

    terminal_printf("Initializing GDT\n");
    init_gdt();
    terminal_printf("GDT Initialized\n");

    terminal_printf("Initializing IDT\n");
    init_idt();
    terminal_printf("IDT Initialized\n");

    init_page_structures();
    map_pages(0, 0, 256); // identity map lower memory
    map_pages(0x200000, 0x200000, 256); // identity map our kernel code and data
    enable_paging();
    terminal_printf("Enabled paging\n");

    terminal_printf("Loading Initial TSS into GDT\n");
    load_tss(&initial_tss);
    terminal_printf("Loaded TSS\n");

    // Testing that paging works. Here we remap an invalid address.
    map_pages(0x694200, 0xFFFFFFFF, 1);
    u8 *ptr = (u16 *) 0xFFFFFFFF; // this wouldn't work without paging

    *ptr = 69;
    terminal_printf("*ptr: %u\n", (u32) *ptr);

    #ifdef TESTS_ENABLED // Test Flag should be passed to build script
        terminal_printf("Charset:\n");
        terminal.color = vga_color_create(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
        for (int h = 0; h < 16; ++h) {
            for (int l = 0; l < 16; ++l) {
                terminal_printf("%c", h << 4 | l);
            }
            terminal_printf("\n");
        }
        terminal.color = vga_color_create(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

        terminal_printf("Colors:\n");
        for (int c = 0; c < 16; ++c) {
            terminal.color = vga_color_create(VGA_COLOR_LIGHT_GREY, c);
            terminal_printf(" ");
        }

        i32 a = -45;
        u32 b = 70;
        const char *message = "Hello!";

        terminal.color = vga_color_create(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        terminal_printf("\n");
        terminal_printf("printf Test: %u, %i, %u, %i, %x, %x, %b, %b, %s, %c, %%\n",
                        a, a, b, b, a, b, a, b, message, *message);
        terminal.color = vga_color_create(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        terminal_printf("\nTesting is enabled!\n");
    #endif

    MMapEntry *entries = mb_info->mmap_addr;

    KERNEL_ASSERT(mb_info->flags & MB_FLAG_MMAP);

    for (u32 i = 0; i < mb_info->mmap_length / sizeof (MMapEntry); i++) {
        KERNEL_ASSERT(entries[i].base_addr_hi == 0 && entries[i].length_hi == 0);
        terminal_printf("%u (%u): %p %u ", i, entries[i].size, entries[i].base_addr_lo, entries[i].length_lo);

        switch (entries[i].type) {
        case MMAP_AVAILABLE:
            terminal_printf("Available\n");
            break;
        case MMAP_RESERVED:
            terminal_printf("Reserved\n");
            break;
        case MMAP_ACPI:
            terminal_printf("ACPI\n");
            break;
        case MMAP_NVS:
            terminal_printf("NVS\n");
            break;
        case MMAP_DEFECTIVE:
            terminal_printf("Defective\n");
            break;
        default:
            terminal_printf("?? (%u)\n", entries[i].type);
            break;
        }
    }


    asm ("ud2");

    for (;;) {
        asm ("hlt");
    }
}
