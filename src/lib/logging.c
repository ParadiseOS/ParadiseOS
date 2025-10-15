#include <stdarg.h>
#include "types.h"
#include "logging.h"
#include "lib/strings.h"
#include "terminal/terminal.h"
#include "drivers/serial/io.h"

static LogLevel LOGLEVEL = INFO;

static char *LEVELS[] = {
    "CRITICAL",
    "INFO",
    "DEBUG"
};

void set_loglevel(LogLevel lvl) {
    LOGLEVEL = lvl;
}

void printk(LogLevel lvl, const char * fmt, ...) {
    if (lvl > LOGLEVEL) return;

    va_list args;
    va_list args_copy;
    va_start(args, fmt);
    va_copy(args_copy, args);

    u32 len        = vsnprintf(NULL, 0, fmt, args_copy);
    u32 prefix_len = snprintf(NULL, 0, "<%s>: ", LEVELS[LOGLEVEL]);
    u32 total_len  = prefix_len + len + 1;

    char buffer[total_len];

    snprintf(buffer, total_len, "<%s>: ", LEVELS[LOGLEVEL]);
    vsnprintf(buffer+prefix_len, total_len-prefix_len, fmt, args);

    char *str = (char *)buffer;
    while(*str){
        terminal_putchar(*str);
        serial_write(*str);
        ++str;
    }
}