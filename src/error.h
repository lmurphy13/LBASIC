/**
 * Error Public Definitions
 * File: error.h
 * Author: Liam M. Murphy
 */

#ifndef ERROR_H
#define ERROR_H

// 2 pages worth of debug message buffer
#define MAX_DEBUG_LEN 4096 * 2
#define MAX_ERROR_LEN 1024

enum exit_error_types {
    LEXER_ERROR_UNKNOWN_CHARACTER = 100,
    LEXER_ERROR_BAD_FILE_POINTER,
    COMPILER_ERROR_UNKNOWN_PATH,
    COMPILER_ERROR_BAD_AST,
    PARSER_ERROR_SYNTAX_ERROR
};

void log_error(const char *msg);
void debug_msg(const char *msg);

#endif // ERROR_H
