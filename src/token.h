/**
 * LBASIC Token Module Public Definitions
 * File: token.h
 * Author: Liam M. Murphy
 */

#ifndef TOKEN_H
#define TOKEN_H

#define MAX_LITERAL 1024

typedef enum {
    T_LPAREN    = 0,  // (
    T_RPAREN    = 1,  // )
    T_ASSIGN    = 2,  // :=
    T_COLON     = 3,  // :
    T_SEMICOLON = 4,  // ;
    T_COMMA     = 5,  // ,
    T_DOT       = 6,  // .
    T_SQUOTE    = 7,  // '
    T_DQUOTE    = 8,  // "
    T_PLUS      = 9,  // +
    T_MINUS     = 10, // -
    T_MUL       = 11, // *
    T_DIV       = 12, // /
    T_MOD       = 13, // %
    T_OFTYPE    = 14, // ->
    T_LT        = 15, // <
    T_GT        = 16, // >
    T_BANG      = 17, // !
    T_EQ        = 18, // ==
    T_LE        = 19, // <=
    T_GE        = 20, // >=
    T_NE        = 21, // !=
    T_AND       = 22, // and
    T_OR        = 23, // or
    T_FUNC      = 24, // func
    T_FOR       = 25, // for
    T_WHILE     = 26, // while
    T_TO        = 27, // to
    T_END       = 28, // end
    T_STRUCT    = 29, // struct
    T_TRUE      = 30, // true
    T_FALSE     = 31, // false
    T_NIL       = 32, // nil
    T_INT       = 33, // int
    T_BOOL      = 34, // bool
    T_STRING    = 35, // string
    T_FLOAT     = 36, // float
    T_VOID      = 37, // void
    T_GOTO      = 38, // goto
    T_IF        = 39, // if
    T_THEN      = 40, // then
    T_ELSE      = 41, // else
    T_RETURN    = 42, // return
    T_IDENT     = 43, // identifier [a-zA-Z] [a-zA-Z0-9]
    T_HEAD      = 44, // Head of list
    L_STR       = 45, // string literal
    L_NUM       = 46, // number literal (integer, for now)
    T_EOF       = 47, // End Of File
    NTOKENS
} token_type;

typedef struct {
    token_type type;
    char literal[MAX_LITERAL];
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
