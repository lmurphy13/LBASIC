/**
 * LBASIC Error Routines
 * File: error.c
 * Author: Liam M. Murphy
 */

#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void log_error(const char *msg) {
    printf("ERROR: %s\n", msg);
    exit(1);
}

void debug_msg(const char *msg) {
#if defined(DEBUG)
    // Append two newlines to the end of msg
    printf("%s\n\n", msg);
#endif
}
