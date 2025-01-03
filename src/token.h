/**
 * LBASIC Token Module Public Definitions
 * File: token.h
 * Author: Liam M. Murphy
 */

#ifndef TOKEN_H
#define TOKEN_H

#include "vector.h" /* line_t, vector */

// Accounting for null byte
#define MAX_LITERAL 1024 + 1

typedef enum {
    T_LPAREN    = 0,  // (
    T_RPAREN    = 1,  // )
    T_LBRACKET  = 2,  // [
    T_RBRACKET  = 3,  // ]
    T_LBRACE    = 4,  // {
    T_RBRACE    = 5,  // }
    T_ASSIGN    = 6,  // :=
    T_COLON     = 7,  // :
    T_SEMICOLON = 8,  // ;
    T_COMMA     = 9,  // ,
    T_DOT       = 10, // .
    T_SQUOTE    = 11, // '
    T_DQUOTE    = 12, // "
    T_PLUS      = 13, // +
    T_MINUS     = 14, // -
    T_MUL       = 15, // *
    T_DIV       = 16, // /
    T_MOD       = 17, // %
    T_OFTYPE    = 18, // ->
    T_LT        = 19, // <
    T_GT        = 20, // >
    T_BANG      = 21, // !
    T_EQ        = 22, // ==
    T_LE        = 23, // <=
    T_GE        = 24, // >=
    T_NE        = 25, // !=
    T_AND       = 26, // and
    T_OR        = 27, // or
    T_FUNC      = 28, // func
    T_FOR       = 29, // for
    T_WHILE     = 30, // while
    T_TO        = 31, // to
    T_END       = 32, // end
    T_STRUCT    = 33, // struct
    T_TRUE      = 34, // true
    T_FALSE     = 35, // false
    T_NIL       = 36, // nil
    T_INT       = 37, // int
    T_BOOL      = 38, // bool
    T_STRING    = 39, // string
    T_FLOAT     = 40, // float
    T_VOID      = 41, // void
    T_GOTO      = 42, // goto
    T_IF        = 43, // if
    T_THEN      = 44, // then
    T_ELSE      = 45, // else
    T_RETURN    = 46, // return
    T_IDENT     = 47, // identifier [a-zA-Z] [a-zA-Z0-9]
    T_HEAD      = 48, // Head of list
    L_STR       = 49, // string literal
    L_INTEGER   = 50, // integer literal
    L_FLOAT     = 51, // float literal
    T_EOF       = 52, // End Of File
    NTOKENS
} token_type;

typedef struct {
    token_type type;
    char literal[MAX_LITERAL];
    char line_str[MAX_LINE];
    unsigned int line;
    unsigned int col;
} token;

typedef struct t_list {
    token *tok;
    struct t_list *prev;
    struct t_list *next;
} t_list;

t_list *t_list_new(void);
void t_list_free(t_list *lst);
void t_list_append(t_list *lst, t_list *new_tok);
t_list *t_list_next(t_list *lst);
t_list *t_list_prev(t_list *lst);
void print_list(t_list *lst);

#endif // TOKEN_H
