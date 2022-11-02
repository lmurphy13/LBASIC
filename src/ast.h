#ifndef AST_H
#define AST_H

#include <stdbool.h>

// Node types
typedef enum n_type {
    N_PROGRAM = 0,
    N_INTEGER,
    N_BOOLEAN,
    N_STRING,
    N_BINOP,
    N_FUNCDECL,
    N_STRUCTDECL,
    N_VARDECL,
    NUM_TYPES
} n_type;

// Root AST node
typedef struct node {
    n_type type;

    union {
        struct {
            int ivalue;
            char *svalue;
        } integer;

        struct {
            bool bvalue;
            char *svalue;
        } boolean;
    };
} node;

// Prototypes
node *mk_node(n_type type);

#endif // AST_H
