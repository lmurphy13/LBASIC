/**
 * Error Public Definitions
 * File: error.h
 * Author: Liam M. Murphy
 */

#ifndef ERROR_H
#define ERROR_H

#include <stdarg.h>

// 2 pages worth of debug message buffer
#define MAX_DEBUG_LEN 4096 * 2
#define MAX_ERROR_LEN 4096 * 2

enum exit_error_types {
    EXIT_GENERIC_ERROR            = 1,
    LEXER_ERROR_UNKNOWN_CHARACTER = 100,
    LEXER_ERROR_BAD_FILE_POINTER,
    COMPILER_ERROR_UNKNOWN_PATH,
    COMPILER_ERROR_BAD_AST,
    PARSER_ERROR_SYNTAX_ERROR,
    TYPE_ERROR
};

void log_error(const char *format, ...);
void debug(const char *format, ...);

#endif // ERROR_H
