/**
 * LBASIC Error Routines
 * File: error.c
 * Author: Liam M. Murphy
 */

#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void log_error(const char *format, ...) {
    printf("[ERROR]: ");

    va_list args;
    va_start(args, format);

    vprintf(format, args);

    va_end(args);

    printf("\n");

    exit(EXIT_GENERIC_ERROR);
}

void debug(const char *format, ...) {
#if defined(DEBUG)
    printf("[DEBUG]: ");

    va_list args;
    va_start(args, format);

    vprintf(format, args);

    va_end(args);

    printf("\n");
#endif
}
