#include <paradise/logging.h>
#include <paradise/serial.h>
#include <paradise/strings.h>
#include <paradise/terminal.h>
#include <paradise/types.h>
#include <paradise/util.h>
#include <stdarg.h>

#if defined(LOG_DEBUG)
static LogLevel LOGLEVEL = DEBUG;
#elif defined(LOG_INFO)
static LogLevel LOGLEVEL = INFO;
#elif defined(LOG_CRITICAL)
static LogLevel LOGLEVEL = CRITICAL;
#else // default log level
static LogLevel LOGLEVEL = DEBUG;
#endif

static char *LEVELS[] = {
    "CRITICAL",
    "INFO",
    "DEBUG",
};

void set_loglevel(LogLevel lvl) {
    LOGLEVEL = lvl;
}

void printk(LogLevel lvl, const char *fmt, ...) {
    if (lvl > LOGLEVEL)
        return;

    va_list args;
    va_start(args, fmt);

    // switch statement to decide color based off level?
    switch (lvl) {
    case 0: {
        terminal.color = vga_color_create(VGA_COLOR_WHITE, VGA_COLOR_RED);
        break;
    }
    case 1: {
        terminal.color = vga_color_create(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
        break;
    }
    default: {
        terminal.color = vga_color_create(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        break;
    }
    }

    u32 prefix_len = snprintf(NULL, 0, "<%s>: ", LEVELS[lvl]);

    snprintf(temp_buffer, TEMP_BUFFER_LENGTH, "<%s>: ", LEVELS[lvl]);
    vsnprintf(
        temp_buffer + prefix_len, TEMP_BUFFER_LENGTH - prefix_len, fmt, args
    );

    u32 idx = 0;
    char *str = (char *) temp_buffer;
    while (*str) {
        if (idx == prefix_len - 1) // switch back to regular terminal color
            terminal.color =
                vga_color_create(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        terminal_putchar(*str);
        serial_write(*str);
        ++str;
        ++idx;
    }
}
