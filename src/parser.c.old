/**
 * LBASIC Syntax Analyzer (Parser)
 * File: parser.c
 * Author: Liam M. Murphy
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "error.h"
#include "parser.h"
#include "token.h"

#define INDENT_WIDTH 4

// Static Prototypes
static token get_token(t_list *);
static token *peek(void);
static void consume(void);
static void backup(void);
static void syntax_error(const char *exp, token l);
static void print_node(node *n, int indent);
static data_type keyword_to_type(token_type t);
static void print_lookahead_debug(const char *msg);

// Parsing Prototypes
void parse_declarations(node *program);
node *parse_declaration(bool *more);
void parse_statements(node *program);
node *parse_statement(bool *more);
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
node *parse_call_expr(void);
node *parse_struct_access_expr(void);
node *parse_for_stmt(void);
node *parse_while_stmt(void);
node *parse_ifthen_stmt(void);
node *parse_ifthenelse_stmt(void);
node *parse_assign_stmt(void);
node *parse_value(void);
node *parse_constant(void);
node *parse_identifier(void);

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

static void syntax_error(const char *exp, token l) {
    printf("Syntax Error (line %d, col %d): Expected %s but got '%s'.\n", l.line, l.col, exp,
           l.literal);
    exit(PARSER_ERROR_SYNTAX_ERROR);
}

// Recursive descent

// <program> := <declarations> <statements>
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

    // Parse declarations
    parse_declarations(program);

    // Parse statements
    parse_statements(program);

    return program;
}

// <declarations> := <declaration> <declarations>
//                 | <declaration>
void parse_declarations(node *program) {
    bool more = true;

    do {
        program->children[program->num_children] = parse_declaration(&more);

        if (more) {
            program->num_children++;
        }
    } while (more);
}

// <declaration> := <function-decl>
//                | <label-decl>
//                | <var-decl>
//                | <struct-decl>
node *parse_declaration(bool *more) {
    node *retval;

    switch (lookahead.type) {
    case T_FUNC:
        retval = parse_function_decl();
        break;
    case T_IDENT: {
        token *tmp = peek();
        // If not colon, this is not a label decl
        if (tmp->type != 3) {
            *more = false;
            break;
        }

        retval = parse_label_decl();
        break;
    }
    case T_INT:
    case T_FLOAT:
    case T_STRING:
    case T_BOOL:
        retval = parse_var_decl();
        break;
    case T_STRUCT:
        retval = parse_struct_decl();
        break;
    default:
        *more = false;
        break;
    }

    return retval;
}

// <statements> := <statement> <statements>
//               | <statement>
void parse_statements(node *program) {
    bool more = true;
    printf("parsing stmts\n");
    do {
        program->children[program->num_children] = parse_statement(&more);

        if (more) {
            program->num_children++;
        }
    } while (more);
}

// <statement> := <for-stmt>
//              | <while-stmt>
//              | <if-then-stmt>
//              | <if-then-else-stmt>
//              | <assign-stmt>
//              | <expression>
node *parse_statement(bool *more) {
    node *retval;
    printf("type: %d\n", lookahead.type);

    while (1)
        ;

    switch (lookahead.type) {
    case T_FOR:
        retval = parse_for_stmt();
        break;
    case T_WHILE:
        retval = parse_while_stmt();
        break;
    case T_IF:
        retval = parse_ifthen_stmt();
        break;
    case N_IFTHENELSE_STMT:
        retval = parse_ifthenelse_stmt();
        break;
    case T_IDENT: {
        print_lookahead_debug("ident");
        token *tmp = peek();
        printf("tmp literal: %s\n", tmp->literal);
        if (strcmp(tmp->literal, ":=") == 0) {
            retval = parse_assign_stmt();
            break;
        } else if (strcmp(tmp->literal, ":") == 0) {
            retval = parse_label_decl();
            break;
        } else {
            printf("Parser Error: Unknown case when encountering N_IDENT\n");
            break;
        }
    }
    case N_EXPR:
        retval = parse_expression();
        break;
    default:
        *more = false;
    }

    return retval;
}

// <function-decl> := 'func' <ident> '(' <formal-list> ')' '->' <type> <statements> 'end'
//                  | 'func' <ident> '(' ')' '->' <type> <statements> 'end'
node *parse_function_decl() {
    node *retval = mk_node(N_FUNC_DECL);
    bool more    = true;

    if (retval != NULL) {
        consume();
        // Add function name to node
        if (lookahead.type == T_IDENT) {
            memset(retval->data.function_decl.name, 0, MAX_LITERAL);
            sprintf(retval->data.function_decl.name, "%s", lookahead.literal);
        } else {
            syntax_error("identifier", lookahead);
        }

        consume();
        if (lookahead.type != T_LPAREN) {
            syntax_error("(", lookahead);
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
            syntax_error("function arguments or ')'", lookahead);
        }

        consume();
        if (lookahead.type != T_OFTYPE) {
            syntax_error("->", lookahead);
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
            syntax_error("a data type", lookahead);
        }

        consume();
        // Begin parsing declarations
        parse_declarations(retval);

        // // Begin parsing statements
        // parse_statements(retval);

        // Parse return statement
        // For now, we will not expect returns in void functions
        if (retval->data.function_decl.type != D_VOID) {
            if (lookahead.type != T_RETURN) {
                syntax_error("'return'", lookahead);
            } else {
                consume();
                retval->data.function_decl.return_expr = parse_expression();
            }
        }

        // Parse the end of the function
        if (lookahead.type != T_END) {
            syntax_error("end", lookahead);
            printf("this one\n");
        } else {
            // Consume 'end'
            consume();
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

    // TODO: Optimize this do-while loop, like when parsing struct member decls.
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
                syntax_error("identifier", *tmp);
            }
            break;
        default:
            // Didn't get a type
            syntax_error("type", lookahead);
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
            f->next   = new;
            new->next = NULL;
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
            first  = false;
        }

    } while (repeat);

    return retval;
}

node *parse_var_decl() {
    node *retval = mk_node(N_VAR_DECL);

    if (retval != NULL) {
        retval->data.var_decl.type = keyword_to_type(lookahead.type);

        consume();
        if (lookahead.type != T_IDENT) {
            syntax_error("type", lookahead);
        } else {
            memset(retval->data.var_decl.name, 0, MAX_LITERAL);
            sprintf(retval->data.var_decl.name, "%s", lookahead.literal);
        }

        consume();
        if (lookahead.type == T_ASSIGN) {
            retval->data.var_decl.value = parse_expression();
        }

        printf("before semicolon\n");
        if (lookahead.type == T_SEMICOLON) {
            // Consume semicolon and we're done
            printf("inside here\n");
            print_lookahead_debug("inside_here");
            consume();
        } else {
            syntax_error("';'", lookahead);
        }
    }

    printf("left var decl\n");

    return retval;
}

node *parse_for_stmt() { return NULL; }
node *parse_ifthen_stmt() { return NULL; }
node *parse_ifthenelse_stmt() { return NULL; }
node *parse_label_decl() { return NULL; }
node *parse_call_expr() { return NULL; }
node *parse_struct_access_expr() { return NULL; }
node *parse_goto_expr() { return NULL; }

// <assign-stmt> := <ident> ( '[' <expression> ']' )? :=' <expression> ';'
node *parse_assign_stmt() {
    printf("inside assign\n");
    node *retval = mk_node(N_ASSIGN);

    if (retval != NULL) {

        // Identifier is either a normal variable or an array access
        // NOTE: Arrays not yet supported
        if (lookahead.type == '[') {
            printf("Parser Error: Arrays not yet supported!");
            exit(1);
        }

        consume();

        if (lookahead.type != T_ASSIGN) {
            syntax_error("':='", lookahead);
        }

        consume();
    }

    return retval;
}

// <while-stmt> := 'while' '(' <expr-lst> ')' <statements> 'end'
node *parse_while_stmt() {
    node *retval = mk_node(N_WHILE_STMT);

    if (retval != NULL) {
        printf("here\n");
        print_lookahead_debug("");

        consume();
        if (lookahead.type != T_LPAREN) {
            syntax_error("'('", lookahead);
        } else {
            print_lookahead_debug("");
            retval->data.while_stmt.test = parse_expression();
            print_lookahead_debug("after parse_expression()");
        }

        if (lookahead.type != T_RPAREN) {
            syntax_error("')'", lookahead);
        }
        consume();
        printf("i am here\n");

        // Decls contained within while stmt children
        parse_declarations(retval);

        print_lookahead_debug("after decls");

        while (1)
            ;

        // Stmts contained within while stmt children
        parse_statements(retval);
        printf("end of while's stmts\n");

        consume();
        if (lookahead.type != T_END) {
            syntax_error("'end'", lookahead);
        } else {
            // Consume past 'end'
            consume();
        }
    }

    return retval;
}

// <struct-decl> := 'struct' <ident> <member-decls> 'end'
node *parse_struct_decl() {
    node *retval = mk_node(N_STRUCT_DECL);

    if (retval != NULL) {
        retval->data.struct_decl.member = NULL;
        if (lookahead.type != T_STRUCT) {
            syntax_error("'struct'", lookahead);
        } else {
            retval->data.struct_decl.type = D_STRUCT;
        }

        consume();
        if (lookahead.type != T_IDENT) {
            syntax_error("identifier", lookahead);
        } else {
            memset(retval->data.struct_decl.name, 0, MAX_LITERAL);
            sprintf(retval->data.struct_decl.name, "%s", lookahead.literal);
        }

        // Parse member declarations
        consume();
        do {
            node *member = mk_node(N_MEMBER_DECL);
            switch (lookahead.type) {
            case T_INT:
            case T_BOOL:
            case T_STRING:
            case T_FLOAT:
                member->data.member_decl.type = keyword_to_type(lookahead.type);
                break;
            default:
                syntax_error("type", lookahead);
            }

            consume();
            if (lookahead.type != T_IDENT) {
                syntax_error("identifier", lookahead);
            } else {
                memset(member->data.member_decl.name, 0, MAX_LITERAL);
                sprintf(member->data.member_decl.name, "%s", lookahead.literal);
            }

            consume();
            if (lookahead.type != T_SEMICOLON) {
                syntax_error("';'", lookahead);
            } else {

                // Seek to end of member list
                node *m = retval->data.struct_decl.member;

                // First member
                if (m == NULL) {
                    retval->data.struct_decl.member       = member;
                    retval->data.struct_decl.member->next = NULL;
                } else {
                    while (m->next != NULL) {
                        m = m->next;
                    }

                    // Append member to decl
                    m->next      = member;
                    member->next = NULL;
                }
            }

            consume();

        } while (lookahead.type != T_END);

        if (lookahead.type != T_END) {
            syntax_error("'end", lookahead);
        } else {
            // Consume 'end'
            consume();
        }
    }

    return retval;
}

// <value> := '(' <expression> ')'
//          | <ident>
//          | <ident> '(' <expr-list> ')'
//          | <constant>
node *parse_value() {
    node *retval = mk_node(N_VALUE);

    if (retval != NULL) {
        switch (lookahead.type) {
        case T_IDENT:
            retval = parse_identifier();
            break;
        case T_LPAREN:
            retval = parse_expression();

            consume();
            if (lookahead.type != T_RPAREN) {
                syntax_error("')'", lookahead);
            }
            break;
        case L_STR:
        case L_NUM:
        case T_TRUE:
        case T_FALSE:
        case T_NIL:
            retval = parse_constant();
            break;
        default:
            syntax_error("value", lookahead);
        }
    }

    return retval;
}

// <constant> := <int-literal>
//             | <float-literal>
//			   | <string-literal>
//			   | 'true'
//			   | 'false'
//			   | 'nil'
node *parse_constant() {
    node *retval = mk_node(N_LITERAL);

    if (retval != NULL) {

        switch (lookahead.type) {
        case L_STR:
            retval->data.literal.type = D_STRING;
            memset(retval->data.literal.value.stringval, 0, MAX_LITERAL);
            sprintf(retval->data.literal.value.stringval, "%s", lookahead.literal);
            break;
        case L_NUM:
            // Only doing integers for now
            retval->data.literal.type         = D_INTEGER;
            retval->data.literal.value.intval = atoi(lookahead.literal);
            break;
        case T_TRUE:
        case T_FALSE:
            retval->data.literal.type = D_BOOLEAN;
            if (strcmp(lookahead.literal, "true") == 0) {
                retval->data.literal.value.boolval = true;
            } else if (strcmp(lookahead.literal, "false") == 0) {
                retval->data.literal.value.boolval = false;
            } else {
                syntax_error("boolean value ('true' or 'false')", lookahead);
            }
            break;
        case T_NIL:
            retval->data.literal.type = D_NIL;
            if (strcmp(lookahead.literal, "nil") == 0) {
                // Like C, we'll consider NIL (NULL) to be zero.
                retval->data.literal.value.intval = 0;
            }
            break;
        default:
            syntax_error("some literal value", lookahead);
        }
        consume();
    }

    return retval;
}

node *parse_identifier() {
    node *retval = mk_node(N_IDENT);

    if (retval != NULL) {
        memset(retval->data.identifier.name, 0, MAX_LITERAL);
        sprintf(retval->data.identifier.name, "%s", lookahead.literal);
    }

    consume();

    return retval;
}

// <expression> := <bin-op-expr>
//               | <goto-expr>
//               | <call-expr>
//               | <struct-access-expr>
node *parse_expression() {
    node *retval;

    consume();
    print_lookahead_debug("inside parse_expression()");

    switch (lookahead.type) {
    case T_GOTO:
        retval = parse_goto_expr();
        break;
    case T_IDENT:
        // On my 32 bit intel system, an empty statement must be here to avoid a compiler error.
        // This error does not occur on my 64 bit system
        ;
        token *tmp = peek();

        // Assignment to a variable
        if (tmp->type == T_SEMICOLON) {
            printf("parsing value\n");
            retval = parse_value();
            break;
        }

        consume();
        if (lookahead.type == T_LPAREN) {
            retval = parse_call_expr();
        } else if (lookahead.type == T_DOT) {
            retval = parse_struct_access_expr();
        } else if (lookahead.type == T_ASSIGN) {
            printf("parsing assignment\n");
            retval = parse_assign_stmt(); // is this the best place?
        } else {
            syntax_error("'(' or '.'", lookahead);
        }
        break;
    // Cheat here to parse constants
    case L_STR:
    case L_NUM:
    case T_TRUE:
    case T_FALSE:
    case T_NIL:
        retval = parse_value();
        break;
    default:
        printf("Other expression types not yet implemented\n");
        exit(1);
        break;
    }

    printf("leaving parse_expression()\n");
    print_lookahead_debug("liam");

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
    case D_NIL:
        return "NIL";
        break;
    default:
        return "UNKNOWN";
        break;
    }
}

static void print_lookahead_debug(const char *msg) {
    if (strlen(msg) > 0) {
        printf("Msg: %s\n", msg);
    }
    printf("Lookahead type: %d\n", lookahead.type);
    printf("Lookahead literal: %s\n", lookahead.literal);
    printf("Line: %d\n", lookahead.line);
    printf("Column: %d\n", lookahead.col);
}

static void print_node(node *n, int indent) {
    if (n == NULL) {
        log_error("Unable to access node for printing\n");
        exit(1);
    }

    print_indent(indent);
    switch (n->type) {
    case N_PROGRAM:
        printf("Program (\n");
        break;
    case N_VAR_DECL:
        printf("VarDecl (\n");
        print_indent(indent + INDENT_WIDTH);
        printf("Name: %s\n", n->data.formal.name);
        print_indent(indent + INDENT_WIDTH);
        printf("Type: %s\n", type_to_str((data_type)n->data.formal.type));
        print_indent(indent + INDENT_WIDTH);
        printf("Value: ");

        indent += INDENT_WIDTH;
        node *v = n->data.var_decl.value;
        if (v != NULL) {
            printf("\n");
            print_node(v, indent + INDENT_WIDTH);
        } else {
            printf("None\n");
        }
        indent -= INDENT_WIDTH;

        print_indent(indent);
        printf("),\n");
        break;
    case N_FUNC_DECL:
        printf("FuncDecl (\n");
        print_indent(indent + INDENT_WIDTH);
        printf("Name: %s\n", n->data.function_decl.name);
        print_indent(indent + INDENT_WIDTH);
        printf("Type: %s\n", type_to_str((data_type)n->data.function_decl.type));
        print_indent(indent + INDENT_WIDTH);
        printf("Formals: ");

        indent += INDENT_WIDTH;
        node *f = n->data.function_decl.formal;
        if (f != NULL) {
            printf("\n");
            while (f != NULL) {
                print_node(f, indent + INDENT_WIDTH);
                f = f->next;
            }
        } else {
            printf("None\n");
        }
        indent -= INDENT_WIDTH;

        // Print FuncDecl children
        for (int idx = 0; idx < n->num_children; idx++) {
            print_node(n->children[idx], indent + INDENT_WIDTH);
        }

        print_indent(indent + INDENT_WIDTH);
        printf("Return: ");
        if (n->data.function_decl.return_expr == NULL) {
            printf("None\n");
        } else {
            print_node(n->data.function_decl.return_expr, indent + INDENT_WIDTH);
        }

        print_indent(indent);
        printf("),\n");
        break;
    case N_STRUCT_DECL:
        printf("StructDecl (\n");
        print_indent(indent + INDENT_WIDTH);
        printf("Name: %s\n", n->data.struct_decl.name);
        print_indent(indent + INDENT_WIDTH);
        printf("Members: ");

        indent += INDENT_WIDTH;
        node *m = n->data.struct_decl.member;
        if (m != NULL) {
            printf("\n");
            while (m != NULL) {
                print_node(m, indent + INDENT_WIDTH);
                m = m->next;
            }
        } else {
            printf("None\n");
        }
        indent -= INDENT_WIDTH;

        print_indent(indent);
        printf("),\n");
        break;
    case N_FORMAL:
        printf("Formal (\n");
        print_indent(indent + INDENT_WIDTH);
        printf("Name: %s\n", n->data.formal.name);
        print_indent(indent + INDENT_WIDTH);
        printf("Type: %s\n", type_to_str((data_type)n->data.formal.type));
        print_indent(indent);
        printf("),\n");
        break;
    case N_MEMBER_DECL:
        printf("MemberDecl (\n");
        print_indent(indent + INDENT_WIDTH);
        printf("Name: %s\n", n->data.member_decl.name);
        print_indent(indent + INDENT_WIDTH);
        printf("Type: %s\n", type_to_str((data_type)n->data.member_decl.type));
        print_indent(indent);
        printf("),\n");
        break;
    case N_LITERAL:
        printf("Literal (\n");
        print_indent(indent + INDENT_WIDTH);
        printf("Type: %s\n", type_to_str((data_type)n->data.literal.type));
        print_indent(indent + INDENT_WIDTH);
        switch (n->data.literal.type) {
        case D_INTEGER:
            printf("Value: %d\n", n->data.literal.value.intval);
            break;
        case D_STRING:
            printf("Value: %s\n", n->data.literal.value.stringval);
            break;
        case D_BOOLEAN:
            printf("Value: %s\n", (n->data.literal.value.boolval) ? "true" : "false");
            break;
        case D_NIL:
            printf("Value: %d (nil)\n", (n->data.literal.value.intval));
            break;
        case D_FLOAT:
            printf("Float literal not yet implemented\n");
            break;
        }
        print_indent(indent);
        printf("),\n");
        break;
    case N_IDENT:
        printf("Identifier (\n");
        print_indent(indent + INDENT_WIDTH);
        printf("Name: %s\n", n->data.identifier.name);
        print_indent(indent);
        printf("),\n");
        break;
    case N_WHILE_STMT:
        printf("WhileStmt (\n");
        print_indent(indent + INDENT_WIDTH);
        printf("Expression: \n");

        indent += INDENT_WIDTH;
        print_node(n->data.while_stmt.test, indent + INDENT_WIDTH);
        indent -= INDENT_WIDTH;

        print_indent(indent);
        printf("),\n");
        break;
    default:
        printf("Unknown node type: %d\n", n->type);
    }
}

void print_ast(node *ast) {
    int indent = 0;

    if (ast != NULL) {
        // Print top level
        print_node(ast, indent);
        indent += INDENT_WIDTH;

        // Print children
        for (int idx = 0; idx < ast->num_children; idx++) {
            node *n = ast->children[idx];
            print_node(n, indent);
        }
    }

    printf(")\n");
    return;
}
