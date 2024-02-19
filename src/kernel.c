#include "terminal.h"
#include "multiboot.h"
#include "init.h"

extern const u32 *multiboot_info;
extern const u32 *stack_top;
extern const u32 *_start;

extern GdtEntry gdt[3];


void kernel_main(void) {
    usize width = multiboot_info[MB_FRAME_BUFFER_WIDTH_OFFSET];
    usize height = multiboot_info[MB_FRAME_BUFFER_HEIGHT_OFFSET];
    u16 *buffer = (u16 * ) multiboot_info[MB_FRAME_BUFFER_ADDR_OFFSET];

    terminal_init(width, height, buffer);


    terminal_printf("Hello, Paradise!\n");
    terminal_printf("Multiboot flags: %x\n", multiboot_info[MB_FLAGS_OFFSET]);
    terminal_printf("Initializing GDT\n");
    init_gdt();
    terminal_printf("GDT Initialized\n");
    

    #ifdef TESTS_ENABLED // Test Flag should be passed to build script
        terminal.color = vga_color_create(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        terminal_printf("\nTesting is enabled!");
    #endif

    for (;;) {
        asm ("hlt");
    }
}
