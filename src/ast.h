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
    N_FOR_STMT,
    N_WHILE_STMT,
    N_IFTHEN_STMT,
    N_IFTHENELSE_STMT,
    N_ASSIGN,
    N_EXPR_LIST,
    N_EXPR,
    N_EMPTY_EXPR,
    N_FORMAL,
    N_BINOP_EXPR,
    N_GOTO_EXPR,
    N_CALL_EXPR,
    N_AND_EXPR,
    N_NOT_EXPR,
    N_COMPARE_EXPR,
    N_ADD_EXPR,
    N_MUL_EXPR,
    N_NEGATE_EXPR,
    N_VALUE,
    N_VALUE_LIST,
    N_IDENT,
    N_IDENT_LIST,
    N_CONSTANT,
    N_LITERAL,
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

// AST node
typedef struct node {
    n_type type;
    union {
        struct {
            vector *statements; // All child nodes within a program will be within this vector
        } program;
        struct {
            struct node *lhs;
            struct node *rhs;
            char operator;
        } bin_op_expr;
        struct {
            data_type type;
            char name[MAX_LITERAL];
        } formal;
        struct {
            data_type type;
            char name[MAX_LITERAL];
        } member_decl;
        struct {
            data_type type;
            char name[MAX_LITERAL];
            struct node *value; // should be an expression node
        } var_decl;
        struct {
            char name[MAX_LITERAL];
        } identifier;
        struct {
            char name[MAX_LITERAL];
            data_type type;
            vector *formals; // formal arguments
            vector *body;    // statements make up the body of a function
            struct node *return_expr;
        } function_decl;
        struct {
            char name[MAX_LITERAL];
            data_type type; // always D_STRUCT
            vector *members;
        } struct_decl;
        struct {
            data_type type;
            union {
                int intval;
                float floatval;
                bool boolval;
                char stringval[MAX_LITERAL];
            } value;
        } literal;
        struct {
            struct node *test;
            vector *body;
        } while_stmt;
    } data;
} node;

// Prototypes
node *mk_node(n_type type);

#endif // AST_H
