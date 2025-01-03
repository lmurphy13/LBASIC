/**
 * Abstract Syntax Tree Public Definitions
 * File: ast.h
 * Author: Liam M. Murphy
 */

#ifndef AST_H
#define AST_H

#include "token.h"
#include "vector.h"
#include <stdbool.h>

#define INDENT_WIDTH 4

// Node types
typedef enum n_type {
    N_PROGRAM            = 0,
    N_STATEMENTS         = 1,
    N_STATEMENT          = 2,
    N_FUNC_DECL          = 3,
    N_LABEL_DECL         = 4,
    N_VAR_DECL           = 5,
    N_MEMBER_DECL        = 6,
    N_STRUCT_DECL        = 7,
    N_BLOCK_STMT         = 8,
    N_FOR_STMT           = 9,
    N_WHILE_STMT         = 10,
    N_IF_STMT            = 11,
    N_RETURN_STMT        = 12,
    N_ARRAY_INIT_EXPR    = 13,
    N_ARRAY_ACCESS_EXPR  = 14,
    N_ASSIGN_EXPR        = 15,
    N_STRUCT_ACCESS_EXPR = 16,
    N_EXPR_LIST          = 17,
    N_EXPR               = 18,
    N_EMPTY_EXPR         = 19,
    N_FORMAL             = 20,
    N_BINOP_EXPR         = 21,
    N_GOTO_STMT          = 22,
    N_CALL_EXPR          = 23,
    N_AND_EXPR           = 24,
    N_NEG_EXPR           = 25,
    N_NOT_EXPR           = 26,
    N_COMPARE_EXPR       = 27,
    N_ADD_EXPR           = 28,
    N_MULT_EXPR          = 29,
    N_PRIMARY_EXPR       = 30,
    N_NEGATE_EXPR        = 31,
    N_VALUE              = 32,
    N_VALUE_LIST         = 33,
    N_IDENT              = 34,
    N_IDENT_LIST         = 35,
    N_CONSTANT           = 36,
    N_INTEGER_LITERAL    = 37,
    N_FLOAT_LITERAL      = 38,
    N_STRING_LITERAL     = 39,
    N_BOOL_LITERAL       = 40,
    N_NIL                = 41,
    NUM_TYPES            = 42
} n_type;

typedef enum data_type {
    D_INTEGER = 0,
    D_FLOAT   = 1,
    D_STRING  = 2,
    D_BOOLEAN = 3,
    D_VOID    = 4,
    D_NIL     = 5,
    D_STRUCT  = 6,
    D_UNKNOWN = 7
} data_type;

typedef struct type_s {
    data_type datatype;
    bool is_function;
    bool is_array;
    char struct_type[MAX_LITERAL];
} type_t;

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

typedef struct label_decl_s {
    char name[MAX_LITERAL];
} label_decl_t;

typedef struct goto_stmt_s {
    char label[MAX_LITERAL];
} goto_stmt_t;

typedef struct call_expr_s {
    char func_name[MAX_LITERAL];
    vector *args; // arguments
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
    char struct_type[MAX_LITERAL];
    bool is_struct;
    bool is_array;
    int num_dimensions;
    char name[MAX_LITERAL];
} formal_t;

// Todo, expand member decls to include arrays or other structs
typedef struct member_decl_s {
    data_type type;
    char name[MAX_LITERAL];
} member_decl_t;

typedef struct var_decl_s {
    data_type type;
    char struct_type[MAX_LITERAL];
    bool is_struct;
    bool is_array;
    int num_dimensions; // keep track of the number of array dimensions
    char name[MAX_LITERAL];
    struct node *value; // should be an expression node
} var_decl_t;

typedef struct function_decl_s {
    char name[MAX_LITERAL];
    data_type type;
    char struct_type[MAX_LITERAL];
    vector *formals;   // formal arguments
    struct node *body; // (block) statements make up the body of a function
    bool is_void;
    bool is_array;
    bool is_struct;
    int num_dimensions;
} function_decl_t;

typedef struct struct_decl_s {
    char name[MAX_LITERAL];
    data_type type;  // always D_STRUCT
    vector *members; // member_decl_t's
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

typedef struct array_init_expr_s {
    vector *expressions;
    /* vector holding expressions for each element of the initialized array.
     * May be a vector of vectors, depending on array dimensionality.
     */
} array_init_expr_t;

typedef struct array_access_expr_s {
    char name[MAX_LITERAL];
    vector *expressions;
    /* vector holding expressions in order of dimension
     * (i.e. my_array[0][i-1][2][3+j] --> 0, i-1, 2, 3+j,
     * where each vector element is an expression)
     *
     * or
     *
     * my_array[0] --> 0
     */
} array_access_expr_t;

typedef struct integer_literal_s {
    data_type type;
    int value;
} integer_literal_t;

typedef struct float_literal_s {
    data_type type;
    float value;
} float_literal_t;

typedef struct string_literal_s {
    data_type type;
    char value[MAX_LITERAL + 1]; // we'll be nice and make these null-terminated
} string_literal_t;

typedef struct bool_literal_s {
    data_type type;
    char str_val[MAX_LITERAL + 1];
    char value; // 0 or 1
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
        block_stmt_t block_stmt;
        while_stmt_t while_stmt;
        if_stmt_t if_stmt;
        return_stmt_t return_stmt;
        assign_expr_t assign_expr;
        neg_expr_t neg_expr;
        not_expr_t not_expr;
        bin_op_expr_t bin_op_expr;
        label_decl_t label_decl;
        goto_stmt_t goto_stmt;
        call_expr_t call_expr;
        struct_access_t struct_access;
        array_init_expr_t array_init_expr;
        array_access_expr_t array_access_expr;
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

data_type keyword_to_type(token_type t);
char *binop_to_str(token_type t);
char *type_to_str(data_type t);

void print_ast(node *ast);
void print_node(node *n, int indent);

#endif // AST_H
