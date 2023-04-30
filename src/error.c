/**
 * LBASIC Error Routines
 * File: error.c
 * Author: Liam M. Murphy
 */

#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void log_error(const char *msg) { printf("ERROR: %s\n", msg); }

void debug_msg(const char *msg) {
#if defined(DEBUG)
    // Append two newlines to the end of msg
    char *debug_buff = (char *)malloc((strlen(msg) + 2) * sizeof(char));
    sprintf(debug_buff, "%s\n\n", msg);

    printf("%s", debug_buff);

    free(debug_buff);
#endif
}
