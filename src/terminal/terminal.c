#include "terminal.h"
#include "drivers/serial/io.h"
#include "lib/strings.h"
#include "lib/util.h"
#include "syscall/syscall.h"
#include <stdarg.h>

Terminal terminal;

u8 vga_color_create(VgaColor fg, VgaColor bg) {
    return bg << 4 | fg;
}

u16 vga_entry_create(u8 c, u8 color) {
    return (u16) color << 8 | (u16) c;
}

void update_cursor() {
    usize pos = terminal.row * terminal.width + terminal.col;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (u8) (pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (u8) ((pos >> 8) & 0xFF));
}

void terminal_scroll(void) {
    for (usize r = 1; r < terminal.height; ++r) {
        for (usize c = 0; c < terminal.width; ++c) {
            terminal.buffer[(r - 1) * terminal.width + c] =
                terminal.buffer[r * terminal.width + c];
        }
    }

    for (usize c = 0; c < terminal.width; ++c) {
        terminal.buffer[(terminal.height - 1) * terminal.width + c] =
            vga_entry_create(' ', terminal.color);
    }

    update_cursor();
}

void terminal_backspace() {
    if (terminal.col == 0)
        return;
    terminal.col--;
    usize i = terminal.row * terminal.width + terminal.col;
    terminal.buffer[i] = vga_entry_create(' ', terminal.color);
    update_cursor();
}

void terminal_putchar(u8 c) {
    if (c == '\n') {
        terminal.col = 0;

        if (++terminal.row == terminal.height) {
            --terminal.row;
            terminal_scroll();
        }
        update_cursor();
        return;
    }

    if (c == '\t') {
        terminal.col += 4;
        update_cursor();
        return;
    }

    if (c == '\b') {
        terminal_backspace();
        return;
    }

    if (terminal.col >= terminal.width) {
        terminal.col = 0;

        if (++terminal.row == terminal.height) {
            --terminal.row;
            terminal_scroll();
        }
    }

    usize i = terminal.row * terminal.width + terminal.col;
    terminal.buffer[i] = vga_entry_create(c, terminal.color);
    ++terminal.col;
    update_cursor();
}

void terminal_print_string(const char *string) {
    for (const char *s = string; *s; ++s) {
        terminal_putchar(*s);
    }
}

// Use sprintf
void terminal_printf(const char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    vsnprintf(temp_string_buffer, STRING_BUFFER_LENGTH, fmt, args);
    terminal_print_string(temp_string_buffer);
}

SyscallResult syscall_print_slice_string(char *s, u32 n) {
    for (u32 i = 0; i < n; i++) {
        terminal_putchar(s[i]);
    }

    SYSCALL_RETURN(0, 0);
}

SyscallResult syscall_print_string(char *s) {
    terminal_printf("%s", s);
    SYSCALL_RETURN(0, 0);
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

    register_syscall(1, syscall_print_slice_string);
    register_syscall(2, syscall_print_string);
}
