#ifndef ERROR_H
#define ERROR_H

enum exit_error_types {
    LEXER_ERROR_UNKNOWN_CHARACTER = 100,
    COMPILER_ERROR_UNKNOWN_PATH   = 101,
    COMPILER_ERROR_BAD_AST        = 102,
    PARSER_ERROR_SYNTAX_ERROR     = 103
};

#endif // ERROR_H
