#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "parser.h"
#include "token.h"

// Static Prototypes
static token get_token(t_list *);
static void consume(void);
static void backup(void);
static void syntax_error(void);

// Parsing Prototypes
node *parse_statements(void);
node *parse_statement(void);
node *parse_function_decl(void);
node *parse_label_decl(void);
node *parse_struct_decl(void);
node *parse_var_decls(void);
node *parse_var_decl(void);
node *parse_expression(void);
node *parse_bin_op_expr(void);
node *parse_and_expr(void);
node *parse_or_expr(void);
node *parse_add_expr(void);
node *parse_sub_expr(void);
node *parse_mul_expr(void);
node *parse_div_expr(void);
node *parse_mod_expr(void);
node *parse_goto_expr(void);

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

static void syntax_error() {
	printf("Syntax Error!\n");
}

static void func_decl() {
	printf("func decl!\n");
}

// Recursive descent

// <program> := <statements>
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

	node *statements = parse_statements();

	program->statements = statements;

	/*
	switch (lookahead.type) {
		case T_FUNC: func_decl(); break;
		default: syntax_error(); break;

	}
	*/

#if defined(DEBUG)
    printf("type: %d\n", lookahead.type);
    printf("literal: %s\n", lookahead.literal);
#endif
    return program;
}

node *parse_statements() {
	return NULL;
}

void print_ast(node *ast) {
    node *root = mk_node(N_PROGRAM);

    printf("(Program (\n");
    printf("         )\n");
    printf(")\n");

    free(root);
    return;
}
