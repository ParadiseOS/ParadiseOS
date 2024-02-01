#include "terminal.h"
#include "multiboot.h"

extern const u32 *multiboot_info;

void kernel_main(void) {
    usize width = multiboot_info[MB_FRAME_BUFFER_WIDTH_OFFSET];
    usize height = multiboot_info[MB_FRAME_BUFFER_HEIGHT_OFFSET];
    u16 *buffer = (u16 * ) multiboot_info[MB_FRAME_BUFFER_ADDR_OFFSET];

    terminal_init(width, height, buffer);

    terminal_write_string("Hello, Paradise!\n");
    terminal_write_string("Multiboot flags: ");
    terminal_print_hex(multiboot_info[MB_FLAGS_OFFSET]);

    terminal_write_string("\nCharset:\n");
    terminal.color = vga_color_create(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
    for (int h = 0; h < 16; ++h) {
        for (int l = 0; l < 16; ++l) {
            terminal_putchar(h << 4 | l);
        }
        terminal_putchar('\n');
    }
    terminal.color = vga_color_create(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

    terminal_write_string("Colors:\n");
    for (int c = 0; c < 16; ++c) {
        terminal.color = vga_color_create(VGA_COLOR_LIGHT_GREY, c);
        terminal_putchar(' ');
    }

    for (;;) {
        asm ("hlt");
    }
}
