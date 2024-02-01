#include "terminal.h"

Terminal terminal;

u8 vga_color_create(VgaColor fg, VgaColor bg) {
    return bg << 4 | fg;
}

u16 vga_entry_create(u8 c, u8 color) {
    return (u16) color << 8 | (u16) c;
}

void terminal_init(usize width, usize height, u16 *buffer) {
    terminal.row = 0;
    terminal.col = 0;
    terminal.width = width;
    terminal.height = height;
    terminal.color = vga_color_create(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal.buffer = buffer;

    for (usize i = 0; i < terminal.width * terminal.height; ++i) {
        terminal.buffer[i] = vga_entry_create(' ', terminal.color);
    }
}

void terminal_scroll() {
    for (usize r = 1; r < terminal.height; ++r) {
        for (usize c = 0; c < terminal.width; ++c) {
            terminal.buffer[(r - 1) * terminal.width + c] = terminal.buffer[r * terminal.width + c];
        }
    }
}

void terminal_putchar(u8 c) {
    if (c == '\n') {
        terminal.col = 0;

        if (++terminal.row == terminal.height) {
            terminal.row--;
            terminal_scroll();
        }

        return;
    }

    usize i = terminal.row * terminal.width + terminal.col;
    terminal.buffer[i] = vga_entry_create(c, terminal.color);

    if (++terminal.col == terminal.width) {
        terminal.col = 0;

        if (++terminal.row == terminal.height) {
            terminal.row = 0;
        }
    }
}

void terminal_write_string(const char *string) {
    for (const char *s = string; *s; ++s) {
        terminal_putchar(*s);
    }
}

const char HEX_DIGITS[] = "0123456789ABCDEF";

void terminal_print_hex(u32 n) {
    for (u32 i = 0; i < 8; ++i) {
        terminal_putchar(HEX_DIGITS[n >> 28]);
        n <<= 4;
    }
}
