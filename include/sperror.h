#ifndef SPERRORS_H
#define SPERRORS_H

#include <stdarg.h>

/**
 * @brief Prints a formatted error message to stderr.
 *
 * This function accepts a format string and a variable number of arguments,
 * similar to printf. The formatted error message is printed to the standard
 * error output.
 *
 * @param fmt The format string (printf-style).
 * @param ... Additional arguments to format.
 */
void spectrel_print_error(const char *fmt, ...);

#endif // SPERRORS_H