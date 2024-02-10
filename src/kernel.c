#include "terminal.h"
#include "multiboot.h"

extern const u32 *multiboot_info;

void kernel_main(void) {
    usize width = multiboot_info[MB_FRAME_BUFFER_WIDTH_OFFSET];
    usize height = multiboot_info[MB_FRAME_BUFFER_HEIGHT_OFFSET];
    u16 *buffer = (u16 * ) multiboot_info[MB_FRAME_BUFFER_ADDR_OFFSET];

    terminal_init(width, height, buffer);

    terminal_printf("Hello, Paradise!\n");
    terminal_printf("Multiboot flags: %b\n", multiboot_info[MB_FLAGS_OFFSET]);

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
    terminal_printf("printf Test: %u, %i, %u, %i, %x, %x, %b, %b, %s, %c, %%",
                    a, a, b, b, a, b, a, b, message, *message);

    #ifdef TESTS_ENABLED // Test Flag should be passed to build script
        terminal_printf("\nTesting is enabled!");
    #endif

    for (;;) {
        asm ("hlt");
    }
}
