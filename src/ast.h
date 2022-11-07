#ifndef AST_H
#define AST_H

#include <stdbool.h>

// Node types
typedef enum n_type {
    N_PROGRAM = 0,
    N_INTEGER,
    N_BOOLEAN,
    N_STRING,
    N_STATEMENTS,
    N_BINOP,
    N_FUNCDECL,
    N_STRUCTDECL,
    N_VARDECL,
    NUM_TYPES
} n_type;

// Program AST node (root node)
typedef struct node {
    n_type type;
    struct node *statements;
} node;

// Prototypes
node *mk_node(n_type type);

#endif // AST_H
