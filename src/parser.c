#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "token.h"

// Globals

node *mk_node(n_type type) {
    node *retval = (node *)malloc(sizeof(node));

    // Freed TBD
    if (retval != NULL) {
        retval->type = type;
    }

    return retval;
}

// Recursive descent
node *parse(t_list *tokens) { return NULL; }

void print_ast(node *ast) {
    node *root = mk_node(N_PROGRAM);

    printf("(Program (\n");
    printf("         )\n");
    printf(")\n");

    free(root);
    return;
}
