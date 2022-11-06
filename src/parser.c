#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "token.h"

// Static Prototypes
static token get_token(t_list *);
static void consume(void);
static void backup(void);

// Globals

// Current token
static token lookahead;

// Pointer to doubly-linked list of tokens
static t_list *toks;

node *mk_node(n_type type) {
    node *retval = (node *)malloc(sizeof(node));

    // Freed TBD
    if (retval != NULL) {
        retval->type = type;
    }

    return retval;
}

// Extracts a token from a t_list struct
static token get_token(t_list *t) {
    token retval;
    retval = *(t->tok);
    return retval;
}

// Advances lookahead by one token
static void consume() {
    t_list *next = t_list_next(toks);
    lookahead    = get_token(next);
    toks         = t_list_next(toks);
}

// Backs-up lookahead by one token
static void backup() {
    t_list *prev = t_list_prev(toks);
    lookahead    = get_token(prev);
    toks         = t_list_prev(toks);
}

// Recursive descent
node *parse(t_list *tokens) {
    node *program = mk_node(N_PROGRAM);

    if (!toks) {
        toks = tokens;
    }

    // Find HEAD token
    lookahead = get_token(toks);

    if (lookahead.type == T_HEAD) {
        // Found head, get first real token
        consume();
    }

    printf("type: %d\n", lookahead.type);
    printf("literal: %s\n", lookahead.literal);

    return program;
}

void print_ast(node *ast) {
    node *root = mk_node(N_PROGRAM);

    printf("(Program (\n");
    printf("         )\n");
    printf(")\n");

    free(root);
    return;
}
