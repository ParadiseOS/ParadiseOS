#include "types.h"
#include "drivers/serial/io.h"
#include "lib/util.h"
#include "lib/strings.h"

#include <stdarg.h>

static const char HEX_DIGITS[] = "0123456789ABCDEF";

static u32 snprint_string(const char *str, u32 n, char *buffer_) {
    u32 len = 0;
    if (str == NULL) {
        str = "(null)";
    }
    for (const char *s = str; *s; s++, len++) {
        if (buffer_ && len < n) {
            buffer_[len] = *s;
        }
    }
    return len;
}

static u32 snprint_hex(u32 val, u32 n, char *buffer_) {
    char temp_buf[8]; // Max 8 hex digits for a 32-bit unsigned int
    int i = 0;

    if (val == 0) {
        if (buffer_ && n > 0) *buffer_ = '0';
        return 1;
    }

    while (val > 0) {
        temp_buf[i++] = HEX_DIGITS[val & 0xF];
        val >>= 4;
    }

    u32 full_len = i;
    if (buffer_) {
        u32 copied = 0;
        while (i > 0 && copied < n) {
            buffer_[copied++] = temp_buf[--i];
        }
    }
    return full_len;
}

static u32 snprint_ptr(void *ptr, u32 n, char *buffer_) {
    u32 val = (u32)(unsigned long)ptr;
    // Calculate full hex length first, passing a max size to ensure full calculation.
    u32 hex_len = snprint_hex(val, ~0u, NULL);
    u32 full_len = 2 + hex_len;

    if (buffer_) {
        u32 written = 0;
        if (written < n) buffer_[written++] = '0';
        if (written < n) buffer_[written++] = 'x';

        if (n > written) {
            snprint_hex(val, n - written, buffer_ + written);
        }
    }
    return full_len;
}

static u32 snprint_bin(u32 val, u32 n, char *buffer_) {
    if (val == 0) {
        if (buffer_ && n > 0) *buffer_ = '0';
        return 1;
    }

    char temp_buf[32];
    int i = 0;
    while (val > 0) {
        temp_buf[i++] = (val & 1) + '0';
        val >>= 1;
    }

    u32 full_len = i;
    if (buffer_) {
        u32 copied = 0;
        while (i > 0 && copied < n) {
            buffer_[copied++] = temp_buf[--i];
        }
    }
    return full_len;
}

static u32 snprint_int(u32 val, bool is_signed, u32 n, char *buffer_) {
    char temp_buffer[12];
    int i = 0;
    bool is_negative = false;

    if (is_signed && (i32)val < 0) {
        val = -(i32)val;
        is_negative = true;
    }

    if (val == 0) {
        temp_buffer[i++] = '0';
    } else {
        while (val) {
            temp_buffer[i++] = (val % 10) + '0';
            val /= 10;
        }
    }

    if (is_negative) {
        temp_buffer[i++] = '-';
    }

    u32 full_len = i;
    if (buffer_) {
        u32 copied = 0;
        while (i > 0 && copied < n) {
            buffer_[copied++] = temp_buffer[--i];
        }
    }
    return full_len;
}

static u32 snprint_float(f64 val, u32 precision, u32 n, char *buffer_) {
    // This function builds the float into a temporary buffer, then uses
    // snprint_string to perform the final size-limited copy.
    char temp_buf[40];
    char *p = temp_buf;
    u32 len = 0;

    if (val != val) return snprint_string("nan", n, buffer_);
    if (val == INFINITY || val == -INFINITY) {
        if (val < 0) *p++ = '-';
        // A bit of manual string copy
        const char *inf = "inf";
        while(*inf) *p++ = *inf++;
        *p = '\0';
        return snprint_string(temp_buf, n, buffer_);
    }

    if (val < 0) {
        *p++ = '-';
        val = -val;
    }

    u32 integer_part = (u32)val;
    len = snprint_int(integer_part, false, sizeof(temp_buf) - (p - temp_buf), p);
    p += len;

    if (precision > 0) {
        *p++ = '.';
        f64 frac_part = val - (f64)integer_part;
        for (u32 i = 0; i < precision; ++i) {
            frac_part *= 10.0;
        }
        len = snprint_int((u32)(frac_part + 0.5), false, sizeof(temp_buf) - (p - temp_buf), p);
        p += len;
    }
    *p = '\0';

    return snprint_string(temp_buf, n, buffer_);
}

// Returns length of would have been string (NOT INCLUDING NULL TERMINATOR)
// Doesn't Write if buffer is NULL
// n includes the NULL Terminator
u32 snprintf(char *buffer, u32 n, const char *fmt, ...) {
    u32 current_length = 0;

    va_list args;
    va_start(args, fmt);

    u32 limit = (n != 0) ? n - 1: 0;

    for (; *fmt; ++fmt) {
        u32 remaining = (current_length > limit) ? 0 : limit - current_length;


        // current_pos is NULL if we have no buffer or are out of space.
        char *current_pos = buffer ? buffer + current_length : NULL;
        if ((u32)current_length >= limit) {
            current_pos = NULL;
        }

        u32 segment_len = 0;

        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case 'u': segment_len = snprint_int(va_arg(args, u32), false, remaining, current_pos); break;
                case 'i': segment_len = snprint_int(va_arg(args, u32), true, remaining, current_pos); break;
                case 'f': segment_len = snprint_float(va_arg(args, f64), 6, remaining, current_pos); break;
                case 'x': segment_len = snprint_hex(va_arg(args, u32), remaining, current_pos); break;
                case 'p': segment_len = snprint_ptr(va_arg(args, void *), remaining, current_pos); break;
                case 'b': segment_len = snprint_bin(va_arg(args, u32), remaining, current_pos); break;
                case 's': segment_len = snprint_string(va_arg(args, char *), remaining, current_pos); break;
                case 'c':
                    if (current_pos) *current_pos = (char)va_arg(args, int);
                    segment_len = 1;
                    break;
                case '%':
                    if (current_pos) *current_pos = '%';
                    segment_len = 1;
                    break;
                case '.': {
                    u32 precision = 0;
                    while (*(fmt + 1) >= '0' && *(fmt + 1) <= '9') {
                        fmt++;
                        precision = precision * 10 + (*fmt - '0');
                    }
                    if (*(fmt + 1) == 'f') {
                        fmt++;
                        segment_len = snprint_float(va_arg(args, f64), precision, remaining, current_pos);
                    }
                    break;
                }
                default: break;
            }
        } else {
            if (current_pos) *current_pos = *fmt;
            segment_len = 1;
        }
        current_length += segment_len;
    }

    va_end(args);

    if (buffer && n > 0) {
        u32 final_pos = ((u32)current_length < limit) ? (u32)current_length : limit;
        buffer[final_pos] = '\0';
    }

    return current_length;
}

i32 strcmp(const char *s1, const char *s2) {
    while (*s1 && *s2 && *s1 == *s2) {
        ++s1;
        ++s2;
    };
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}