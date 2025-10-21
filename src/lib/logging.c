#include "logging.h"
#include "drivers/serial/io.h"
#include "lib/strings.h"
#include "terminal/terminal.h"
#include "types.h"
#include <stdarg.h>

#define MAX_LOG_LENGTH 8192

#ifdef LOG_DEBUG
static LogLevel LOGLEVEL = DEBUG;
#endif

#ifdef LOG_INFO
static LogLevel LOGLEVEL = INFO;
#endif

#ifdef LOG_CRITICAL
static LogLevel LOGLEVEL = CRITICAL;
#endif

static char LOG_BUFFER[MAX_LOG_LENGTH];

static char *LEVELS[] = {"CRITICAL", "INFO", "DEBUG"};

void set_loglevel(LogLevel lvl) {
    LOGLEVEL = lvl;
}

void printk(LogLevel lvl, const char *fmt, ...) {
    if (lvl > LOGLEVEL)
        return;

    va_list args;
    va_start(args, fmt);

    u32 prefix_len = snprintf(NULL, 0, "<%s>: ", LEVELS[lvl]);

    snprintf(LOG_BUFFER, MAX_LOG_LENGTH, "<%s>: ", LEVELS[lvl]);
    vsnprintf(LOG_BUFFER + prefix_len, MAX_LOG_LENGTH - prefix_len, fmt, args);

    char *str = (char *) LOG_BUFFER;
    while (*str) {
        terminal_putchar(*str);
        serial_write(*str);
        ++str;
    }
}
