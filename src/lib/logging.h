#ifndef LOGGING_H_
#define LOGGING_H_

typedef enum { CRITICAL = 0, INFO = 1, DEBUG = 2 } LogLevel;

/**
 * @brief Sets the loglevel fro the terminal (LOWER ~ HIGHER PRECIDENCE NEEDED)
 * @param lvl precidence of the message to be logged
 */
void set_loglevel(LogLevel lvl);

/**
 * @brief Outputs to serial logging & terminal
 * if the set lvl has enough precidence
 *
 * @param lvl precidence of the message to be logged
 * @param fmt The format string.
 * @param ... The variable arguments for the format string.
 */
void printk(LogLevel lvl, const char *fmt, ...);

#endif