/**
 * LBASIC Syntax Analyzer (Parser)
 * File: parser.c
 * Author: Liam M. Murphy
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "error.h"
#include "parser.h"
#include "token.h"

// Static Prototypes
static token get_token(t_list *);
static token *peek(void);
static void consume(void);
static void backup(void);
static void syntax_error(const char *msg, const char *got, unsigned int line);
static void print_node(node *n, int indent);
static data_type keyword_to_type(token_type t);

// Parsing Prototypes
node *parse_statements(void);
node *parse_statement(void);
node *parse_function_decl(void);
node *parse_formals(void);
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
        // Assign type
        retval->type = type;

        // Zero out list of children nodes
        memset(retval->children, 0, sizeof(retval->children));
        retval->num_children = 0;

        retval->next = NULL;
    }

    return retval;
}

// Extracts a token from a t_list struct
static token get_token(t_list *t) {
    token retval;
    retval = *(t->tok);
    return retval;
}

// Looks ahead by one token, but does not consume it
static token *peek() {
    t_list *next = t_list_next(toks);
    return next->tok;
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

static void syntax_error(const char *exp, const char *got, unsigned int line) {
    printf("Syntax Error (line %d): Expected %s but got '%s'.\n", line, exp, got);
    exit(1);
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

    switch (lookahead.type) {
    case T_FUNC:
        program->children[program->num_children] = parse_function_decl();
        program->num_children++;
        break;
    default:
        printf("Unknown token type: %d\n", lookahead.type);
        break;
    }

    return program;
}

// <statements> := <statement> <statements>
//               | <statement>
node *parse_statements() { return NULL; }

// <statement> := <function-decl>
//              | <label-decl>
//              | <var-decl>
//              | <struct-decl>
//              | <for-stmt>
//              | <while-stmt>
//              | <if-then-stmt>
//              | <if-then-else-stmt>
//              | <expression>
node *parse_statement() {
    
}

// <function-decl> := 'func' <ident> '(' <formal-list> ')' '->' <type> <statements> 'end'
//                  | 'func' <ident> '(' ')' '->' <type> <statements> 'end'
node *parse_function_decl() {
    node *retval = mk_node(N_FUNC_DECL);
    if (retval != NULL) {
        consume();
        // Add function name to node
        if (lookahead.type == T_IDENT) {
            memset(retval->data.function_decl.name, 0, MAX_LITERAL);
            sprintf(retval->data.function_decl.name, "%s", lookahead.literal);
        } else {
            syntax_error("identifier", lookahead.literal, lookahead.line);
        }

        consume();
        if (lookahead.type != T_LPAREN) {
            syntax_error("(", lookahead.literal, lookahead.line);
        }

        token *tmp = peek();

        // List of formal args
        if (tmp->type != T_RPAREN) {
            retval->data.function_decl.formal = parse_formals();
            consume();
            tmp = peek();
        }
        
        // No formal args, expect a )
        else if (tmp->type == T_RPAREN) {
            consume();
        } else {
            syntax_error("function arguments or ')'", lookahead.literal, lookahead.line);
        }

        consume();
        if (lookahead.type != T_OFTYPE) {
            syntax_error("->", lookahead.literal, lookahead.line);
        }

        consume();
        switch (lookahead.type) {
        case T_INT:
            retval->data.function_decl.type = D_INTEGER;
            break;
        case T_BOOL:
            retval->data.function_decl.type = D_BOOLEAN;
            break;
        case T_STRING:
            retval->data.function_decl.type = D_STRING;
            break;
        case T_FLOAT:
            retval->data.function_decl.type = D_FLOAT;
            break;
        case T_VOID:
            retval->data.function_decl.type = D_VOID;
            break;
        default:
            // Expected a type, but didn't get one
            syntax_error("a data type", lookahead.literal, lookahead.line);
        }

        // Begin parsing statements
        consume();
        retval->data.function_decl.statements = parse_statements();

        // Parse the end of the function
        consume();
        if (lookahead.type != T_END) {
            syntax_error("end", lookahead.literal, lookahead.line);
        }
    } else {
        printf("mk_node failed for N_FUNC_DECL\n");
    }

    return retval;
}

// <formal-list> := <formal> ',' <formal-list>
//                | <formal>
//
// <formal> := <type> <ident>
//
// I am cheating a bit here and parsing the list all in one go, instead of creating
// a separate function for the <formal-list> non-terminal.
node *parse_formals() {
    bool repeat = true;
    bool first  = true;

    node *retval = mk_node(N_FORMAL);
    node *new    = {0};

    do {
        if (!first) {
            new = mk_node(N_FORMAL);
        }

        // Lookahead should be a type
        consume();

        // Expecting an identifier
        token *tmp = peek();

        switch (lookahead.type) {
        case T_INT:
        case T_BOOL:
        case T_STRING:
        case T_FLOAT:
            if (tmp->type != T_IDENT) {
                syntax_error("identifier", tmp->literal, tmp->line);
            }
            break;
        default:
            // Didn't get a type
            syntax_error("type", lookahead.literal, lookahead.line);
        }

        if (first) {
            retval->data.formal.type = keyword_to_type(lookahead.type);
        } else {
            new->data.formal.type = keyword_to_type(lookahead.type);
        }

        // Now should be the identifier itself
        consume();

        if (first) {
            memset(retval->data.formal.name, 0, MAX_LITERAL);
            sprintf(retval->data.formal.name, "%s", lookahead.literal);
        } else {
            memset(new->data.formal.name, 0, MAX_LITERAL);
            sprintf(new->data.formal.name, "%s", lookahead.literal);
        }

        if (first) {
            retval->next = NULL;
        } else {
            node *f = retval;

            // Seek to end of list
            while (f->next != NULL) {
                f = f->next;
            }

            // Append new formal to end of list
            f->next = new;
            new->next    = NULL;
        }

        // Look for a ',' or ')'
        tmp = peek();

        // End of formals
        if (tmp->type == T_RPAREN) {
            repeat = false;
            break;
        }

        // More formals
        else if (tmp->type == T_COMMA) {
            consume();
            repeat = true;
            first = false;
        }

    } while (repeat);

    return retval;
}

static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf(" ");
    }
}

static data_type keyword_to_type(token_type t) {
    switch (t) {
        case T_INT:
            return D_INTEGER;
            break;
        case T_BOOL:
            return D_BOOLEAN;
            break;
        case T_STRING:
            return D_STRING;
            break;
        case T_FLOAT:
            return D_FLOAT;
            break;
        case T_VOID:
            return D_VOID;
            break;
        default:
            return D_UNKNOWN;
            break;
    }
}

static const char *type_to_str(data_type t) {
    switch (t) {
    case D_INTEGER:
        return "INTEGER";
        break;
    case D_FLOAT:
        return "FLOAT";
        break;
    case D_STRING:
        return "STRING";
        break;
    case D_BOOLEAN:
        return "BOOLEAN";
        break;
    case D_VOID:
        return "VOID";
        break;
    default:
        return "UNKNOWN";
        break;
    }
}

static void print_node(node *n, int indent) {
    print_indent(indent);
    switch (n->type) {
    case N_PROGRAM:
        printf("Program (\n");
        break;
    case N_FUNC_DECL:
        printf("FuncDecl (\n");
        print_indent(indent + 4);
        printf("Name: %s\n", n->data.function_decl.name);
        print_indent(indent + 4);
        printf("Type: %s\n", type_to_str((data_type)n->data.function_decl.type));
        print_indent(indent + 4);
        printf("Formals: ");
        indent += 4;
        node *f = n->data.function_decl.formal;
        if (f != NULL) {
            printf("\n");
            while (f != NULL) {
                print_node(f, indent + 4);
                f = f->next;
            }
        } else {
            printf("None\n");
        }
        indent -= 4;
        print_indent(indent);
        printf("),\n");
        break;
    case N_FORMAL:
        printf("Formal (\n");
        print_indent(indent + 4);
        printf("Name: %s\n", n->data.formal.name);
        print_indent(indent + 4);
        printf("Type: %s\n", type_to_str((data_type)n->data.formal.type));
        print_indent(indent);
        printf("),\n");
        break;
    }
}

void print_ast(node *ast) {
    int indent = 0;
    print_node(ast, indent);
    indent += 4;
    while (ast != NULL) {
        node *func = ast->children[0];
        print_node(func, indent);
        break;
    }

    printf(")\n");
    return;
}
