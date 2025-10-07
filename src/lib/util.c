#include "util.h"
#include "types.h"
#include <stdarg.h>

#include "drivers/serial/io.h"
#include "lib/util.h"

#define BUF_SIZE 1024

extern void write1(const char *str, int sz);
extern void write2(const char *str);

// Helper function for pprintf
i8 print_string(const char *str, char *buffer_) {
    i8 len = 0;
    for (const char *s = str; *s; s++, len++) {
        *buffer_++ = *s;
    }
    return len;
}

const char HEX_DIGITS[] = "0123456789ABCDEF";

// Helper function for pprintf
i8 print_hex(u32 n, char *buffer_) {
    i8 len = 0;
    bool leading_zeros = true;

    if (n == 0) {
        *buffer_++ = '0';
        return 1;
    }

    for (i8 i = 7; i >= 0; --i) {
        u32 nibble = (n >> (i * 4)) & 0xF;

        if (leading_zeros && nibble == 0 && i > 0) {
            continue;
        }

        leading_zeros = false;
        *buffer_++ = HEX_DIGITS[nibble];
        len++;
    }

    return len;
}

// Helper function for pprintf
i8 print_ptr(void *ptr, char *buffer_) {
    i8 len = 0;

    // Write "0x" to the buffer
    *buffer_++ = '0';
    *buffer_++ = 'x';
    len += 2;

    u32 n = (u32) ptr;
    len += print_hex(n, buffer_);

    return len;
}

// Helper function for pprintf
i8 print_bin(u32 n, char *buffer_) {
    i8 len = 0;
    bool leading_zeros = true;

    // Handle the special case of a zero value
    if (n == 0) {
        *buffer_++ = '0';
        return 1;
    }

    // A u32 has 32 bits, so we loop 32 times
    for (i8 i = 31; i >= 0; --i) {
        // Extract the bit at position i
        u8 bit = (n >> i) & 1;

        // Skip leading zeros
        if (leading_zeros && bit == 0) {
            continue;
        }

        leading_zeros = false;
        *buffer_++ = bit + '0';
        len++;
    }

    return len;
}

// Helper function for pprintf
i8 print_int(u32 n, bool is_signed, char *buffer_) {
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

    i8 len = i;
    while (i > 0) {
        *buffer_++ = buffer[--i];
    }

    return len;
}

// Helper function for pprintf
i8 print_float(f64 n, u32 precision, char *buffer_) {
    i8 len = 0;

    if (n != n) {
        len = print_string("nan", buffer_);
        return len;
    }

    if (n == INFINITY) {
        if (n < 0) {
            *buffer_++ = '-';
            len++;
        }
        len += print_string("inf", buffer_ + len);
        return len;
    }

    if (n < 0) {
        *buffer_++ = '-';
        len++;
        n = -n;
    }

    u32 integer = (u32) n;
    len += print_int(integer, false, buffer_ + len);
    buffer_ += len;

    if (precision > 0) {
        *buffer_++ = '.';
        len++;

        f64 fraction_float = n - (f64) integer;
        for (u32 i = 0; i < precision; i++) {
            fraction_float *= 10.0;
        }

        u32 fraction_int = (u32) (fraction_float + 0.5); // Rounding

        u32 temp_fraction_int = fraction_int;
        i8 num_printed = 0;
        for (u32 i = 0; i < precision; i++) {
            if (temp_fraction_int == 0) {
                *buffer_++ = '0';
                len++;
                num_printed++;
            }
            else {
                break;
            }
        }

        char temp_buffer[20];
        i8 i = 0;
        if (fraction_int == 0) {
            if (num_printed == 0) {
                for (u32 k = 0; k < precision; k++) {
                    *buffer_++ = '0';
                    len++;
                }
            }
        }
        else {
            while (fraction_int > 0) {
                temp_buffer[i++] = (fraction_int % 10) + '0';
                fraction_int /= 10;
            }
            while (i > 0) {
                *buffer_++ = temp_buffer[--i];
                len++;
            }
        }
    }

    return len;
}

void pputc(const char c) {
    write1(&c, 1);
}

void pputs(const char *str) {
    write2(str);
}

void pputn(const char *str, u32 length) {
    write1(str, length);
}

void pprintf(const char *fmt, ...) {
    char buffer[BUF_SIZE];
    int len = 0;

    va_list args;
    va_start(args, fmt);

    for (; *fmt; ++fmt) {
        char c = *fmt;

        if (c == '%') {
            ++fmt;

            switch (*fmt) {
            case 'u':
                len += print_int(va_arg(args, u32), false, buffer + len);
                break;
            case 'i':
                len += print_int(va_arg(args, u32), true, buffer + len);
                break;
            case 'f':
                len += print_float(va_arg(args, f64), 6, buffer + len);
                break;
            case 'x':
                len += print_hex(va_arg(args, u32), buffer + len);
                break;
            case 'p':
                len += print_ptr(va_arg(args, void *), buffer + len);
                break;
            case 'b':
                len += print_bin(va_arg(args, u32), buffer + len);
                break;
            case 's':
                len += print_string(va_arg(args, char *), buffer + len);
                break;
            case 'c':
                buffer[len++] = va_arg(args, u32);
                break;
            case '%':
                buffer[len++] = '%';
                break;
            case '.': {
                u32 precision = 0;
                while (*(fmt + 1) >= '0' && *(fmt + 1) <= '9') {
                    fmt++;
                    precision = precision * 10 + (*fmt - '0');
                }

                if (*(fmt + 1) == 'f') {
                    fmt++;
                    len +=
                        print_float(va_arg(args, f64), precision, buffer + len);
                    break;
                }
            }
            default:
                break;
            }
        }
        else {
            buffer[len++] = c;
        }
    }

    if (len < BUF_SIZE) {
        buffer[len] = '\0';
    }

    write1(buffer, len);

    va_end(args);
}