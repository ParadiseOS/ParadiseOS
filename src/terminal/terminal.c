#include "terminal.h"
#include "drivers/serial/io.h"
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
    serial_write(c);

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

static const char HEX_DIGITS[] = "0123456789ABCDEF";

void terminal_print_hex(u32 n) {
    if (n == 0) {
        terminal_putchar('0');
        return;
    }

    u32 msb = 31 - __builtin_clz(n);
    u32 msn = (msb + 4) & ~3; // most significant nibble
    u32 i = 0;
    n <<= 32 - msn; // put the msn at the most significant index

    do {
        terminal_putchar(HEX_DIGITS[n >> 28]);
        n <<= 4;
        i += 4;
    } while (i < msn);
}

void terminal_print_ptr(void *ptr) {
    u32 n = (u32) ptr;

    terminal_putchar('0');
    terminal_putchar('x');

    for (u32 i = 0; i < 8; ++i) {
        terminal_putchar(HEX_DIGITS[n >> 28]);
        n <<= 4;
    }
}

void terminal_print_bin(u32 n) {
    if (n == 0) {
        terminal_putchar('0');
        return;
    }

    u32 num_leading_zeros = __builtin_clz(n);
    u32 msb = 32 - num_leading_zeros;
    u32 i = 0;
    n <<= num_leading_zeros; // put the msb at the most significant index

    do {
        terminal_putchar((n >> 31) + '0');
        n <<= 1;
        ++i;
    } while (i < msb);
}

void terminal_print_int(u32 n, bool is_signed) {
    char buffer[12];
    i8 i = 0;

    bool is_negative = false;

    if (is_signed && (i32) n < 0) {
        n = -n;
        is_negative = true;
    }

    if (!n) {
        buffer[i++] = '0';
    }
    else {
        while (n) {
            buffer[i++] = (n % 10) + '0';
            n /= 10;
        }
    }

    if (is_negative) {
        buffer[i++] = '-';
    }

    for (i = i - 1; i >= 0; --i) {
        terminal_putchar(buffer[i]);
    }
}

void terminal_print_float(f64 n, u32 precision) {
    if (n != n) {
        terminal_print_string("nan");
        return;
    }

    if (n == INFINITY) {
        terminal_print_string("inf");
        return;
    }

    if (n == NEG_INFINITY) {
        terminal_print_string("-inf");
        return;
    }

    if (n < 0) {
        terminal_putchar('-');
        n = -n;
    }

    u32 integer = (u32) n;
    terminal_print_int(integer, false);

    if (precision == 0)
        return;

    terminal_putchar('.');

    f64 fraction_float = n - (f64) integer;

    f64 multiplier = 1.0;
    for (u32 i = 0; i < precision; i++)
        multiplier *= 10.0;

    u32 fraction_int = (u32) (fraction_float * multiplier + 0.5);
    // 0.5 for rounding

    char buffer[precision + 1];
    i8 i = 0;
    for (u32 j = 0; j < precision; j++) {
        buffer[i++] = (fraction_int % 10) + '0';
        fraction_int /= 10;
    }

    for (i = i - 1; i >= 0; i--) {
        terminal_putchar(buffer[i]);
    }
}

void terminal_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    for (; *fmt; ++fmt) {
        char c = *fmt;

        if (c == '%') {
            ++fmt;

            switch (*fmt) {
            case 'u':
                terminal_print_int(va_arg(args, u32), false);
                break;
            case 'i':
                terminal_print_int(va_arg(args, u32), true);
                break;
            case 'f':
                terminal_print_float(va_arg(args, f64), 6);
                break;
            case 'p':
                terminal_print_ptr(va_arg(args, void *));
                break;
            case 'x':
                terminal_print_hex(va_arg(args, u32));
                break;
            case 'b':
                terminal_print_bin(va_arg(args, u32));
                break;
            case 's':
                terminal_print_string(va_arg(args, char *));
                break;
            case 'c':
                terminal_putchar(va_arg(args, u32));
                break;
            case '%':
                terminal_putchar('%');
                break;
            case '.': {
                u32 precision = 0;
                while (*(fmt + 1) >= '0' && *(fmt + 1) <= '9') {
                    fmt++;
                    precision = precision * 10 + (*fmt - '0');
                }

                if (*(fmt + 1) == 'f') {
                    fmt++;
                    terminal_print_float(va_arg(args, f64), precision);
                    break;
                }
            }
            default:
                break;
            }
        }
        else {
            terminal_putchar(c);
        }
    }
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
