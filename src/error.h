/**
 * Error Definitions
 * File: error.h
 * Author: Liam M. Murphy
 */

#ifndef ERROR_H
#define ERROR_H

enum exit_error_types {
    LEXER_ERROR_UNKNOWN_CHARACTER = 100,
    LEXER_ERROR_BAD_FILE_POINTER,
    COMPILER_ERROR_UNKNOWN_PATH,
    COMPILER_ERROR_BAD_AST,
    PARSER_ERROR_SYNTAX_ERROR
};

#endif // ERROR_H
