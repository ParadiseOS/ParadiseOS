#include "logging.h"
#include "drivers/serial/io.h"
#include "lib/strings.h"
#include "lib/util.h"
#include "terminal/terminal.h"
#include "types.h"
#include <stdarg.h>

static LogLevel LOGLEVEL = DEBUG;

#ifdef LOG_DEBUG
static LogLevel LOGLEVEL = DEBUG;
#endif

#ifdef LOG_INFO
static LogLevel LOGLEVEL = INFO;
#endif

#ifdef LOG_CRITICAL
static LogLevel LOGLEVEL = CRITICAL;
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

    u32 prefix_len = snprintf(NULL, 0, "<%s>: ", LEVELS[lvl]);

    snprintf(temp_buffer, TEMP_BUFFER_LENGTH, "<%s>: ", LEVELS[lvl]);
    vsnprintf(
        temp_buffer + prefix_len, TEMP_BUFFER_LENGTH - prefix_len, fmt, args
    );

    char *str = (char *) temp_buffer;
    while (*str) {
        terminal_putchar(*str);
        serial_write(*str);
        ++str;
    }
}
