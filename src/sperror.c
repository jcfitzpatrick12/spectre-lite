
#include "sperror.h"
#include <stdarg.h>
#include <stdio.h>

void spectrel_print_error(const char *fmt, ...)
{
    va_list args;
    fprintf(stderr, "Error: ");
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}
