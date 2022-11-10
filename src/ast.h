/**
 * Abstract Syntax Tree Definitions
 * File: ast.h
 * Author: Liam M. Murphy
 */

#ifndef AST_H
#define AST_H

#include <stdbool.h>
#include "token.h"

#define MAX_CHILDREN 1024

// Node types
typedef enum n_type {
    N_PROGRAM = 0,
	N_STATEMENTS,
	N_STATEMENT,
	N_FUNC_DECL,
	N_LABEL_DECL,
	N_VAR_DECL,
	N_STRUCT_DECL,
	N_FOR_STMT,
	N_WHILE_STMT,
	N_IFTHEN_STMT,
	N_IFTHENELSE_STMT,
	N_EXPR_LIST,
	N_EXPR,
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
    NUM_TYPES
} n_type;

typedef enum data_type {
	D_INTEGER = 0,
	D_FLOAT,
	D_STRING,
	D_BOOLEAN,
	D_VOID
} data_type;

// Root AST node
typedef struct node {
    n_type type;
   	union {
		struct {
			char name[MAX_LITERAL];
			data_type return_type;
			struct node *statements;
			struct node *args;	// formal arguments
		} function_decl;
	} data;
	struct node *children[MAX_CHILDREN];
	int num_children;
} node;

// Prototypes
node *mk_node(n_type type);

#endif // AST_H
