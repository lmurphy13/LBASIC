/**
 * Abstract Syntax Tree Public Definitions
 * File: ast.h
 * Author: Liam M. Murphy
 */

#ifndef AST_H
#define AST_H

#include "token.h"
#include "utils.h"
#include <stdbool.h>

// Node types
typedef enum n_type {
    N_PROGRAM = 0,
    N_STATEMENTS,
    N_STATEMENT,
    N_FUNC_DECL,
    N_LABEL_DECL,
    N_VAR_DECL,
    N_MEMBER_DECL,
    N_STRUCT_DECL,
    N_BLOCK_STMT,
    N_FOR_STMT,
    N_WHILE_STMT,
    N_IF_STMT,
    N_RETURN_STMT,
    N_ASSIGN_EXPR,
    N_EXPR_LIST,
    N_EXPR,
    N_EMPTY_EXPR,
    N_FORMAL,
    N_BINOP_EXPR,
    N_GOTO_EXPR,
    N_CALL_EXPR,
    N_AND_EXPR,
    N_NEG_EXPR,
    N_NOT_EXPR,
    N_COMPARE_EXPR,
    N_ADD_EXPR,
    N_MULT_EXPR,
    N_PRIMARY_EXPR,
    N_NEGATE_EXPR,
    N_VALUE,
    N_VALUE_LIST,
    N_IDENT,
    N_IDENT_LIST,
    N_CONSTANT,
    N_LITERAL,
    N_INTEGER_LITERAL,
    N_FLOAT_LITERAL,
    N_STRING_LITERAL,
    N_BOOL_LITERAL,
    N_NIL,
    NUM_TYPES
} n_type;

typedef enum data_type {
    D_INTEGER = 0,
    D_FLOAT,
    D_STRING,
    D_BOOLEAN,
    D_VOID,
    D_NIL,
    D_STRUCT,
    D_UNKNOWN
} data_type;

// Node types
typedef struct program_s {
    vector *statements; // All child nodes within a program will be within this vector
} program_t;

// Multi-use block of statements (function body, if-else body, etc.)
typedef struct block_stmt_s {
    vector *statements;
} block_stmt_t;

typedef struct identifier_s {
    char name[MAX_LITERAL];
} identifier_t;

typedef struct literal_s {
    data_type type;
    union {
        int intval;
        float floatval;
        bool boolval;
        char stringval[MAX_LITERAL];
    } value;
} literal_t;

typedef struct neg_expr_s {
    struct node *expr;
} neg_expr_t;

typedef struct not_expr_s {
    struct node *expr;
} not_expr_t;

typedef struct bin_op_expr_s {
    struct node *lhs;
    struct node *rhs;
    token_type operator;
} bin_op_expr_t;

typedef struct goto_expr_s {
    char label[MAX_LITERAL];
} goto_expr_t;

typedef struct call_expr_s {
    char name[MAX_LITERAL];
    vector *formals; // formal args
} call_expr_t;

typedef struct struct_access_s {
    char name[MAX_LITERAL];
    char member_name[MAX_LITERAL];
} struct_access_t;

typedef struct assign_expr_s {
    struct node *lhs;
    struct node *rhs;
} assign_expr_t;

typedef struct formal_s {
    data_type type;
    char name[MAX_LITERAL];
} formal_t;

typedef struct member_decl_s {
    data_type type;
    char name[MAX_LITERAL];
} member_decl_t;

typedef struct var_decl_s {
    data_type type;
    char name[MAX_LITERAL];
    struct node *value; // should be an expression node
} var_decl_t;

typedef struct function_decl_s {
    char name[MAX_LITERAL];
    data_type type;
    vector *formals;   // formal arguments
    struct node *body; // (block) statements make up the body of a function
    bool is_void;
} function_decl_t;

typedef struct struct_decl_s {
    char name[MAX_LITERAL];
    data_type type; // always D_STRUCT
    vector *members;
} struct_decl_t;

typedef struct while_stmt_s {
    struct node *test;
    struct node *body;
} while_stmt_t;

typedef struct if_stmt_s {
    struct node *test;
    struct node *body;
    struct node *else_stmt;
} if_stmt_t;

typedef struct return_stmt_s {
    struct node *expr;
} return_stmt_t;

typedef struct integer_literal_s {
    int value;
} integer_literal_t;

typedef struct float_literal_s {
    float value;
} float_literal_t;

typedef struct string_literal_s {
    char value[MAX_LITERAL + 1]; // we'll be nice and make these null-terminated
} string_literal_t;

typedef struct bool_literal_s {
    char str_val[6]; // enough space to fit "false" plus null terminator
    char value;      // 0 or 1
} bool_literal_t;

typedef struct nil_s {
    char value; // Always zero
} nil_t;

// AST node
typedef struct node {
    n_type type;
    union {
        program_t program;
        formal_t formal;
        member_decl_t member_decl;
        var_decl_t var_decl;
        function_decl_t function_decl;
        struct_decl_t struct_decl;
        literal_t literal;
        block_stmt_t block_stmt;
        while_stmt_t while_stmt;
        if_stmt_t if_stmt;
        return_stmt_t return_stmt;
        assign_expr_t assign_expr;
        neg_expr_t neg_expr;
        not_expr_t not_expr;
        bin_op_expr_t bin_op_expr;
        goto_expr_t goto_expr;
        call_expr_t call_expr;
        struct_access_t struct_access;
        identifier_t identifier;
        integer_literal_t integer_literal;
        float_literal_t float_literal;
        string_literal_t string_literal;
        bool_literal_t bool_literal;
        nil_t nil;
    } data;
} node;

// Prototypes
node *mk_node(n_type type);

#endif // AST_H
