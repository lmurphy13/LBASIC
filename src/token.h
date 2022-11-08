#ifndef TOKEN_H
#define TOKEN_H

#define MAX_LITERAL 1024

typedef enum {
    T_LPAREN = 0, // (
    T_RPAREN,     // )
    T_ASSIGN,     // :=
    T_COLON,      // :
    T_SEMICOLON,  // ;
    T_SQUOTE,     // '
    T_DQUOTE,     // "
    T_PLUS,       // +
    T_MINUS,      // -
    T_MUL,        // *
    T_DIV,        // /
    T_MOD,        // %
    T_OFTYPE,     // ->
    T_LT,         // <
    T_GT,         // >
    T_BANG,       // !
    T_EQ,         // ==
    T_LE,         // <=
    T_GE,         // >=
    T_NE,         // !=
    T_AND,        // and
    T_OR,         // or
    T_FUNC,       // func
    T_FOR,        // for
    T_WHILE,      // while
    T_TO,         // to
    T_END,        // end
    T_STRUCT,     // struct
    T_TRUE,       // true
    T_FALSE,      // false
    T_NIL,        // nil
    T_INT,        // int
    T_BOOL,       // bool
    T_STRING,     // string
    T_FLOAT,      // float
    T_GOTO,       // goto
    T_IDENT,      // identifier [a-zA-Z] [a-zA-Z0-9]
    T_HEAD,       // Head of list
    L_STR,        // string literal
    L_NUM,        // number literal
    T_EOF,        // End Of File
    NTOKENS
} token_type;

typedef struct {
    token_type type;
    char literal[MAX_LITERAL];
} token;

typedef struct t_list {
    token *tok;
    struct t_list *prev;
    struct t_list *next;
} t_list;

t_list *t_list_new(void);
void t_list_append(t_list *lst, t_list *new_tok);
t_list *t_list_next(t_list *lst);
t_list *t_list_prev(t_list *lst);
void print_list(t_list *lst);

#endif // TOKEN_H
