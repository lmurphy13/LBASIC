/**
 * LBASIC Syntax Analyzer (Parser)
 * File: parser.c
 * Author: Liam M. Murphy
 */

#include "parser.h"

#include "ast.h"
#include "error.h"
#include "token.h"
#include "utils.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INDENT_WIDTH 4
#define N_BUILTINS 1

// Globals
static token lookahead;

// List of "builtin" functions (identifiers)
static char builtins[N_BUILTINS][MAX_LITERAL] = {"println"};

// Pointer to doubly-linked list of tokens
static t_list *toks;

// Private prototypes
static token get_token(t_list *);
static token *peek(void);
static void consume(void);
static void backup(void);
static void syntax_error(const char *exp, token l);
static void print_node(node *n, int indent);
static data_type keyword_to_type(token_type t);
static void print_lookahead_debug(const char *msg);
static bool is_builtin(const char *ident);
static bool is_binop_symbol(token *t);

// Grammar productions
static vector *parse_statements(void);    // done
static node *parse_statement(bool *more); // done
static node *parse_block_stmt(void);      // done
static node *parse_for_stmt(void);        // TBD later on
static node *parse_while_stmt(void);      // done
static node *parse_if_stmt(void);         // done
static node *parse_assign_expr(void);     // done
static vector *parse_arg_list(void);      // done
static node *parse_call_expr(void);       // done
static node *parse_expression(void);      // done
static vector *parse_formals(void);       // done
static node *parse_function_decl(void);   // done
static node *parse_label_decl(void);      // done
static node *parse_goto_stmt(void);       // done
static node *parse_var_decl(void);        // done
static node *parse_member_decl(void);     // done
static node *parse_struct_decl(void);     // done
static node *parse_struct_access_expr(void);
static node *parse_return_stmt(void); // done

/* Precedence rules, lowest to highest
 * && ||
 * !
 * == != > >= < <=
 * + -
 * * / %
 */
static node *parse_and_expr(void);     // done
static node *parse_not_expr(void);     // done
static node *parse_compare_expr(void); // done
static node *parse_add_expr(void);     // done
static node *parse_mult_expr(void);    // done
// Same as a 'factor'. Here we parse primitives.
static node *parse_primary_expr(void); // done

static node *parse_identifier(void);      // done
static node *parse_string_literal(void);  // done
static node *parse_integer_literal(void); // done
static node *parse_float_literal(void);   // done
static node *parse_bool_literal(void);    // done
static node *parse_nil(void);             // done

node *mk_node(n_type type) {
    node *retval = (node *)malloc(sizeof(node));
    memset(retval, 0, sizeof(retval));

    // Freed TBD
    if (retval != NULL) {
        // Assign type
        retval->type = type;

        if (type == N_PROGRAM) {
            retval->data.program.statements = mk_vector();

            if (retval->data.program.statements == NULL) {
                log_error("Could not allocate node's 'statements' vector");
                exit(1);
            }
        }
    } else {
        log_error("mk_node(): Unable to allocate memory for new AST node");
        exit(1);
    }

    return retval;
}

// Extracts a token from a t_list struct
static token get_token(t_list *t) {
    token retval;
    if (t != NULL) {
        retval = *(t->tok);
    } else {
        log_error("Failed to get next token. Bad t_list or you're trying to access beyond the end "
                  "of the token list.");
        exit(1);
    }
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
    printf("Syntax Error (line %d, col %d): Expected '%s' but got '%s'.\n", l.line, l.col, exp,
           l.literal);
    printf("%s", l.line_str);
    for (int i = 0; i < l.col; i++) {
        printf(" ");
    }
    printf("^\n");
    exit(PARSER_ERROR_SYNTAX_ERROR);
}

// Check to see if an identifier (string) is a builtin function
static bool is_builtin(const char *ident) {
    bool found = false;

    for (int i = 0; i < N_BUILTINS; i++) {
        if (strcmp(ident, builtins[i]) == 0) {
            found = true;
            break;
        }
    }

    return found;
}

static bool is_binop_symbol(token *t) {
    bool ret = false;

    if (t != NULL) {
        switch (t->type) {
        case T_PLUS:
        case T_MINUS:
        case T_MUL:
        case T_DIV:
        case T_MOD:
            ret = true;
            break;
        default:
            assert(0 && "Other binop symbol types not yet implemented");
        }
    }

    return ret;
}

static void print_lookahead_debug(const char *msg) {
#ifdef DEBUG
    if (strlen(msg) > 0) {
        printf("Msg: %s\n", msg);
    }
    printf("Lookahead type: %d\n", lookahead.type);
    printf("Lookahead literal: %s\n", lookahead.literal);
    printf("Line: %d\n", lookahead.line);
    printf("Column: %d\n", lookahead.col);
#endif
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

        if (lookahead.type == T_EOF) {
            // If we go immediately to an EOF, this is an empty file.
            log_error("Empty files are not valid LBASIC programs");
        }
    }

    // Parse the body of the program
    program->data.program.statements = parse_statements();

    return program;
}

// <statements> := <statement> <statements>
//               | <statement>
static vector *parse_statements() {
    vector *retval = mk_vector();
    bool more      = true;
    printf("parsing stmts\n");

    if (retval != NULL) {
        do {
            node *new_node = parse_statement(&more);
            if (new_node != NULL) {
                print_lookahead_debug("adding statement node");
                printf("NODE TYPE: %d\n", new_node->type);
                vector_add(retval, new_node);

                // If we reach the end of the file, break out
                if (lookahead.type == T_EOF) {
                    break;
                }
            }
        } while (more);
    } else {
        log_error("parse_statements(): Unable to allocate vector");
    }

    return retval;
}

// <statement> := <block-stmt>
//              | <for-stmt>
//              | <while-stmt>
//              | <if-stmt>
//              | <if-else-stmt>
//              | <assign-stmt>
//              | <function-decl>
//              | <var-decl>
//              | <label-decl>
//              | <goto-stmt>
//              | <struct-decl>
//              | <expression>
//              | ';' (empty statement)
//              | ( <expression> )
static node *parse_statement(bool *more) {
    node *retval = NULL;
    printf("type: %d\n", lookahead.type);

    switch (lookahead.type) {
    case T_THEN:
        retval = parse_block_stmt();
        break;
    case T_FOR:
        retval = parse_for_stmt();
        break;
    case T_WHILE:
        retval = parse_while_stmt();
        break;
    case T_IF:
        retval = parse_if_stmt();
        break;
    case T_FUNC:
        retval = parse_function_decl();
        break;
    case T_IDENT: {
        print_lookahead_debug("ident");
        token *tmp = peek();
        printf("tmp literal: %s\n", tmp->literal);

        if (strcmp(tmp->literal, ":=") == 0) {
            retval = parse_assign_expr();
            break;
        } else if (strcmp(tmp->literal, ":") == 0) {
            retval = parse_label_decl();
            break;
        } else if (strcmp(tmp->literal, "(") == 0) {
            // Likely a function call
            retval = parse_expression();
            break;
        } else if ((strcmp(tmp->literal, "and") == 0) || (strcmp(tmp->literal, "or") == 0) ||
                   (strcmp(tmp->literal, "+") == 0) || (strcmp(tmp->literal, "-") == 0) ||
                   (strcmp(tmp->literal, "*") == 0) || (strcmp(tmp->literal, "/") == 0) ||
                   (strcmp(tmp->literal, "%") == 0) || (strcmp(tmp->literal, ">") == 0) ||
                   (strcmp(tmp->literal, "<") == 0) || (strcmp(tmp->literal, ">=") == 0) ||
                   (strcmp(tmp->literal, "<=") == 0) || (strcmp(tmp->literal, "==") == 0) ||
                   (strcmp(tmp->literal, "!=") == 0)) {
            retval = parse_expression(); // binop exprs when dealing with variables
            break;
        } else if (strcmp(tmp->literal, ";") == 0) {
            // Maybe we'll make this a no-op situation, but for now just raise an error
            char str[1024] = {'\0'};
            snprintf(str, sizeof(str), "Illegal statement: %s%s (line %d, col: %d)",
                     lookahead.literal, tmp->literal, tmp->line, tmp->col);
            log_error(str);
        } else if (strcmp(tmp->literal, ".") == 0) {
            retval = parse_struct_access_expr();
            break;
        } else {
            log_error("Parser Error: Unknown case when encountering N_IDENT\n");
            break;
        }
    }
    case T_GOTO:
        retval = parse_goto_stmt();
        break;
    case T_INT:
    case T_FLOAT:
    case T_STRING:
    case T_BOOL:
        retval = parse_var_decl();
        break;
    case L_INTEGER:
    case L_FLOAT:
        retval = parse_expression();
        break;
    case T_STRUCT: {
        // We are either a struct declaration or a variable declaration
        // This is our only LL(2) region
        token *next1 = peek();
        if (next1->type == T_IDENT) {
            consume();
            token *next2 = peek();

            if (next2->type == T_IDENT) {
                // If we see 'struct <ident> <ident>', then this is a struct variable declaration.
                backup();

                retval = parse_var_decl();
            } else if (next2->type == T_THEN) {
                // If we see 'struct <ident> then', then this is a struct declaration.
                backup();

                retval = parse_struct_decl();
            } else {
                // Some other case, which is an error
                syntax_error("identifier or 'then'", lookahead);
            }
        }
        break;
    }
    case T_LPAREN:
        retval = parse_expression();
        break;
    case T_RETURN:
        retval = parse_return_stmt();
        break;
    case T_SEMICOLON:
        // If we see a lone semicolon, just consume it and move on. Empty statement.
        printf("FOUND LONE SEMICOLON\n");
        consume();
        break;
    case T_END: // This should be the end of most bodies (conditionals, functions, etc)
    case T_EOF: // End of file, we're done
    default:
        *more = false;
    }

    return retval;
}

// <block-stmt> := 'then' <statements>
static node *parse_block_stmt() {
    node *retval = mk_node(N_BLOCK_STMT);

    if (retval != NULL) {
        // Look for 'then'
        if (lookahead.type == T_THEN) {
            // Consume it
            consume();
        } else {
            syntax_error("then", lookahead);
        }

        // Parse statements until we hit 'end'
        // NOTE: There will be nested 'end's (if's, loops)
        retval->data.block_stmt.statements = parse_statements();
    }

    return retval;
}

static node *parse_for_stmt() { assert(0 && "Not yet implemented"); }

// <while-stmt> := 'while' '(' <expression> ')' <block-stmt> 'end'
static node *parse_while_stmt() {
    node *retval = mk_node(N_WHILE_STMT);

    if (retval != NULL) {
        // Parse 'while'
        if (lookahead.type != T_WHILE) {
            syntax_error("while", lookahead);
        }
        // Consume while
        consume();

        // Parse beginning of test '('
        if (lookahead.type != T_LPAREN) {
            syntax_error("(", lookahead);
        }
        // Consume (
        consume();

        // Parse expression
        retval->data.while_stmt.test = parse_expression();

        // Parse ending ')'
        if (lookahead.type != T_RPAREN) {
            syntax_error(")", lookahead);
        }
        // Consume )
        consume();

        // Parse body
        retval->data.while_stmt.body = parse_block_stmt();

        // Look for 'end'
        if (lookahead.type != T_END) {
            syntax_error("end", lookahead);
        } else {
            consume();
        }
    }

    return retval;
}

// <if-stmt> := 'if' '(' <expression> ')' <block-stmt> ('else' 'then' <block-stmt>)? 'end'
static node *parse_if_stmt() {
    node *retval = mk_node(N_IF_STMT);

    if (retval != NULL) {
        // Parse 'if'
        if (lookahead.type != T_IF) {
            syntax_error("if", lookahead);
        }
        // Consume if
        consume();

        // Parse beginning of test '('
        if (lookahead.type != T_LPAREN) {
            syntax_error("(", lookahead);
        }
        // Consume (
        consume();

        // Parse expression
        retval->data.if_stmt.test = parse_expression();

        // Parse ending ')'
        if (lookahead.type != T_RPAREN) {
            syntax_error(")", lookahead);
        }
        // Consume )
        consume();

        // Parse body
        retval->data.if_stmt.body = parse_block_stmt();

        // If we see an 'end' token, there will be no 'else'
        if (lookahead.type == T_END) {
            retval->data.if_stmt.else_stmt = NULL;
            consume();
        } else if (lookahead.type == T_ELSE) {
            consume();
            retval->data.if_stmt.else_stmt = parse_block_stmt();

            if (lookahead.type == T_END) {
                consume();
            } else {
                syntax_error("end", lookahead);
            }
        } else {
            syntax_error("else or end", lookahead);
        }
    }

    return retval;
}

// <if-stmt> := 'if' '(' <expression> ')' 'then' <stmts> ('else' <if-stmt>)* <end>
// To handle the else-if we have to do this a bit differently

// And and Or
static node *parse_and_expr() {
    print_lookahead_debug("begin and_expr");

    // Try to parse the "left hand side"
    node *retval     = NULL;
    node *e1         = parse_not_expr(); // "Term"
    node *e2         = NULL;
    token_type ttype = -1;

    switch (lookahead.type) {
    case T_AND:
    case T_OR:
        ttype = lookahead.type;
        break;
    }

    // We didn't find an operator, so this is a "leaf" expression. No "right hand side"
    if (ttype == -1) {
        retval = e1;
    } else {
        // Consume the operator
        consume();
        e2 = parse_not_expr(); // "Term"

        if (e1 != NULL) {
            if (e2 != NULL) {
                retval = mk_node(N_BINOP_EXPR);
                if (retval != NULL) {
                    retval->data.bin_op_expr.lhs     = e1;
                    retval->data.bin_op_expr.rhs     = e2;
                    retval->data.bin_op_expr.operator= ttype;
                }
            } else {
                log_error("parse_and_expr(): e2 is NULL");
            }
        } else {
            log_error("parse_and_expr(): e1 is NULL");
        }
    }

    return retval;
}

// Negation and unary minus
static node *parse_not_expr() {
    print_lookahead_debug("begin not_expr");

    node *retval = NULL;

    // Look for !
    if (lookahead.type == T_BANG) {
        print_lookahead_debug("found !");
        // Consume it
        consume();

        // Parse the expression
        retval = mk_node(N_NOT_EXPR);
        if (retval != NULL) {
            retval->data.not_expr.expr = parse_expression();
        } else {
            log_error("Unable to create N_NOT_EXPR node");
        }
    } else if (lookahead.type == T_MINUS) {
        consume();
        print_lookahead_debug("found -");
        // Parse the expr
        retval = mk_node(N_NEG_EXPR);
        if (retval != NULL) {
            retval->data.neg_expr.expr = parse_expression();
        } else {
            log_error("Unable to create N_NEG_EXPR node");
        }
    } else {
        print_lookahead_debug("did not find !");
        // If we didn't find a !, continue to parse.
        retval = parse_compare_expr();
    }

    return retval;
}

// Comparisons (relational)
static node *parse_compare_expr() {
    print_lookahead_debug("begin compare_expr");

    // Try to parse the "left hand side"
    node *retval     = NULL;
    node *e1         = parse_add_expr(); // "Term"
    node *e2         = NULL;
    token_type ttype = -1;

    switch (lookahead.type) {
    case T_EQ:
    case T_NE:
    case T_GT:
    case T_GE:
    case T_LT:
    case T_LE:
        ttype = lookahead.type;
        break;
    }

    // We didn't find an operator, so this is a "leaf" expression. No "right hand side"
    if (ttype == -1) {
        retval = e1;
    } else {
        // Consume the operator
        consume();
        e2 = parse_add_expr(); // "Term"

        if (e1 != NULL) {
            if (e2 != NULL) {
                retval = mk_node(N_BINOP_EXPR);
                if (retval != NULL) {
                    retval->data.bin_op_expr.lhs     = e1;
                    retval->data.bin_op_expr.rhs     = e2;
                    retval->data.bin_op_expr.operator= ttype;
                }
            } else {
                log_error("parse_add_expr(): e2 is NULL");
            }
        } else {
            log_error("parse_add_expr(): e1 is NULL");
        }
    }

    return retval;
}

// Addition and subtraction
static node *parse_add_expr() {
    print_lookahead_debug("begin add_expr");

    // Try to parse the "left hand side"
    node *retval     = NULL;
    node *e1         = parse_mult_expr(); // "Term"
    node *e2         = NULL;
    token_type ttype = -1;

    switch (lookahead.type) {
    case T_PLUS:
    case T_MINUS:
        ttype = lookahead.type;
        break;
    }

    // We didn't find an operator, so this is a "leaf" expression. No "right hand side"
    if (ttype == -1) {
        retval = e1;
    } else {
        // Consume the operator
        consume();
        e2 = parse_mult_expr(); // "Term"

        if (e1 != NULL) {
            if (e2 != NULL) {
                retval = mk_node(N_BINOP_EXPR);
                if (retval != NULL) {
                    retval->data.bin_op_expr.lhs     = e1;
                    retval->data.bin_op_expr.rhs     = e2;
                    retval->data.bin_op_expr.operator= ttype;
                }
            } else {
                log_error("parse_add_expr(): e2 is NULL");
            }
        } else {
            log_error("parse_add_expr(): e1 is NULL");
        }
    }

    return retval;
}

// Multiplication, division, and modulus
static node *parse_mult_expr() {
    print_lookahead_debug("begin mult_expr");

    // Try to parse the "left hand side"
    node *retval     = NULL;
    node *e1         = parse_primary_expr(); // "Factor"
    node *e2         = NULL;
    token_type ttype = -1;

    switch (lookahead.type) {
    case T_MUL:
    case T_DIV:
    case T_MOD:
        ttype = lookahead.type;
        break;
    }

    // We didn't find an operator, so this is a "leaf" expression. No "right hand side"
    if (ttype == -1) {
        retval = e1;
    } else {
        // Consume the operator
        consume();
        e2 = parse_primary_expr(); // "Factor"

        if (e1 != NULL) {
            if (e2 != NULL) {
                retval = mk_node(N_BINOP_EXPR);
                if (retval != NULL) {
                    retval->data.bin_op_expr.lhs     = e1;
                    retval->data.bin_op_expr.rhs     = e2;
                    retval->data.bin_op_expr.operator= ttype;
                }
            } else {
                log_error("parse_mult_expr(): e2 is NULL");
            }
        } else {
            log_error("parse_mult_expr(): e1 is NULL");
        }
    }

    return retval;
}

// Literals and grouping expressions
static node *parse_primary_expr() {
    node *retval;
    bool parsed_func_call = false;
    print_lookahead_debug("begin primary_expr");

    // Move past assignment operator
    //    consume();

    switch (lookahead.type) {
    case L_INTEGER:
        retval = parse_integer_literal();
        break;
    case L_FLOAT:
        retval = parse_float_literal();
        break;
    case L_STR:
        retval = parse_string_literal();
        break;
    case T_TRUE:
    case T_FALSE:
        retval = parse_bool_literal();
        break;
    case T_IDENT: {
        // We need to check if this is a regular variable or a function call
        token *tmp = peek();
        if ((tmp != NULL) && (tmp->type == T_LPAREN)) {
            // This is a function call
            retval = parse_call_expr();
            print_lookahead_debug("returned from call_expr");
            return retval;
        } else if (tmp != NULL) {
            // Treat this as a regular variable name
            retval = parse_identifier();
        }
        break;
    }
    case T_NIL:
        retval = parse_nil();
        break;
    case T_LPAREN:
        consume();
        retval = parse_expression();
        consume();
        return retval;
        break;
    }

    // Consume the literal or ident
    consume();
    print_lookahead_debug("end primary_expr");

    return retval;
}

// <assign-expr> := <ident> ( '[' <expression> ']' )? ':=' <expression> ';'
static node *parse_assign_expr() {
    node *retval = mk_node(N_ASSIGN_EXPR);

    if (retval != NULL) {
        // Identifier should be live in
        if (lookahead.type == T_IDENT) {
            retval->data.assign_expr.lhs = parse_identifier();
        } else {
            syntax_error("identifier", lookahead);
        }

        // Consume identifier (ignoring arrays for now)
        consume();

        print_lookahead_debug("after consuming ident");

        // Now we should be looking at an assignment operator
        if (lookahead.type == T_ASSIGN) {
            // Consume assignment
            consume();
        } else {
            syntax_error(":=", lookahead);
        }

        print_lookahead_debug("before parsing RHS");

        // Right hand side is an expression
        retval->data.assign_expr.rhs = parse_expression();

        print_lookahead_debug("after parsing RHS");

        if (lookahead.type == T_SEMICOLON) {
            consume();
        } else {
            syntax_error(";", lookahead);
        }
    }

    return retval;
}

// <arg-list> := ( <expression> (',')? )*
static vector *parse_arg_list() {
    vector *retval = mk_vector();

    if (retval == NULL) {
        log_error("Unable to allocate vector for function call arguments");
    }

    print_lookahead_debug("inside parse_arg_list");

    // Look for args (expressions)
    bool repeat = true;

    node *new_arg_expr = NULL;

    do {
        new_arg_expr = parse_expression();
        if (new_arg_expr != NULL) {
            vector_add(retval, new_arg_expr);
        } else {
            log_error("Unable to parse argument expression");
        }

        print_lookahead_debug("after argument");

        if (lookahead.type == T_RPAREN) {
            repeat = false;
        } else if (lookahead.type == T_COMMA) {
            consume();
            repeat = true;
        }
    } while (repeat);

    return retval;
}

// <call-expr> := <identifier> '(' ( <arg-list> )? ')'
static node *parse_call_expr() {
    node *retval = mk_node(N_CALL_EXPR);

    if (retval != NULL) {
        print_lookahead_debug("inside call_expr");
        if (lookahead.type != T_IDENT) {
            syntax_error("identifier", lookahead);
        }

        memset(retval->data.call_expr.func_name, 0, sizeof(retval->data.call_expr.func_name));
        snprintf(retval->data.call_expr.func_name, sizeof(retval->data.call_expr.func_name), "%s",
                 lookahead.literal);

        // Consume function name
        consume();

        if (lookahead.type != T_LPAREN) {
            syntax_error("(", lookahead);
        }

        // Consume (
        consume();

        if (lookahead.type == T_RPAREN) {
            // No args
            retval->data.call_expr.args = NULL;
        } else {
            retval->data.call_expr.args = parse_arg_list();

            if (lookahead.type != T_RPAREN) {
                syntax_error(") after argument list", lookahead);
            }
            consume();
        }

        // Check for something that might indicate we are not a lone function call
        bool found_follower = false;
        switch (lookahead.type) {
        case T_RPAREN:
        case T_COMMA:
        case T_PLUS:
        case T_MINUS:
        case T_MUL:
        case T_DIV:
        case T_MOD:
        case T_LT:
        case T_GT:
        case T_EQ:
        case T_LE:
        case T_GE:
        case T_NE:
        case T_AND:
        case T_OR:
            found_follower = true;
        }

        // If we don't find any followers, check for a semicolon
        if (!found_follower) {
            if (lookahead.type != T_SEMICOLON) {
                syntax_error("; after function call", lookahead);
            } else {
                consume();
            }
        }
    }

    print_lookahead_debug("end of call_expr");

    return retval;
}

static vector *parse_formals() {
    print_lookahead_debug("inside parse_formals()");
    bool repeat = true;
    bool first  = true;

    vector *retval = mk_vector();

    node *formal = mk_node(N_FORMAL);
    node *new    = {0};

    // TODO: Optimize this do-while loop, like when parsing struct member decls.
    do {
        if (!first) {
            new = mk_node(N_FORMAL);
        }

        consume();
        // Lookahead is now a type

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
            formal->data.formal.type = keyword_to_type(lookahead.type);
        } else {
            new->data.formal.type = keyword_to_type(lookahead.type);
        }

        // Now should be the identifier itself
        consume();

        if (first) {
            memset(formal->data.formal.name, 0, MAX_LITERAL);
            sprintf(formal->data.formal.name, "%s", lookahead.literal);
            vector_add(retval, formal);
        } else {
            memset(new->data.formal.name, 0, MAX_LITERAL);
            sprintf(new->data.formal.name, "%s", lookahead.literal);
            vector_add(retval, new);
        }

        // Look for a ',' or ')'
        tmp = peek();

        // End of formals
        if (tmp->type == T_RPAREN) {
            consume();
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

static node *parse_function_decl() {
    node *retval = mk_node(N_FUNC_DECL);

    if (retval != NULL) {
        // Look for 'func'
        if (lookahead.type == T_FUNC) {
            consume();
        } else {
            syntax_error("func", lookahead);
        }

        // Look for identifier
        if (lookahead.type == T_IDENT) {
            snprintf(retval->data.function_decl.name, sizeof(retval->data.function_decl.name), "%s",
                     lookahead.literal);
            consume();
        } else {
            syntax_error("function name", lookahead);
        }

        // Look for formal args
        if (lookahead.type == T_LPAREN) {
            token *tok = peek();
            if ((tok != NULL) && (tok->type == T_RPAREN)) {
                // No args
                retval->data.function_decl.formals = NULL;

                // Consume '('
                consume();

                // Consume ')'
                consume();
            } else if (tok != NULL) {
                // Maybe we have some formals

                // consume the (
                // consume();
                retval->data.function_decl.formals = parse_formals();

                // Consume the ')' at the end of the formals list
                consume();
            }
        } else {
            syntax_error("(", lookahead);
        }

        // Look for type arrow
        if (lookahead.type == T_OFTYPE) {
            consume();
        } else {
            syntax_error("->", lookahead);
        }

        // Look for type
        switch (lookahead.type) {
        case T_INT:
        case T_FLOAT:
        case T_BOOL:
        case T_STRING:
        case T_VOID:
            retval->data.function_decl.type = keyword_to_type(lookahead.type);
            break;
        default:
            syntax_error("type declaration", lookahead);
            break;
        }

        consume();

        // Look for 'then'
        if (lookahead.type == T_THEN) {
            // If we have one, parse the function body
            retval->data.function_decl.body = parse_block_stmt();
        } else {
            syntax_error("then", lookahead);
        }

        // Parse the end token and we're done
        if (lookahead.type == T_END) {
            consume();
        } else {
            syntax_error("end", lookahead);
        }

        print_lookahead_debug("here");
    }

    return retval;
}

static node *parse_expression() {
    struct node *retval;
    print_lookahead_debug("begin parse_expr()");

    switch (lookahead.type) {
    case T_IDENT:
    case L_INTEGER:
    case L_FLOAT:
    case L_STR:
    case T_TRUE:
    case T_FALSE:
    case T_LPAREN:
    case T_BANG:
    case T_MINUS:
        // Entry point into arithmetic expressions and booleans
        retval = parse_and_expr();
        break;
    case T_ASSIGN:
        retval = parse_assign_expr();
        break;
    default:
        print_lookahead_debug("default parse_expr()");
        assert(0 && "Expression type not yet implemented");
    }

    return retval;
}

// <label-decl> := <identifier> ':'
static node *parse_label_decl() {
    node *retval = mk_node(N_LABEL_DECL);

    if (retval != NULL) {
        // Look for label name
        if (lookahead.type != T_IDENT) {
            syntax_error("identifier", lookahead);
        } else {
            memset(retval->data.label_decl.name, 0, MAX_LITERAL);
            snprintf(retval->data.label_decl.name, sizeof(retval->data.label_decl.name), "%s",
                     lookahead.literal);
        }

        consume();

        // Look for colon
        if (lookahead.type != T_COLON) {
            syntax_error(":", lookahead);
        }

        // Consume it
        consume();
    }

    return retval;
}

// <goto-stmt> := 'goto' <identifier> ';'
static node *parse_goto_stmt() {
    node *retval = mk_node(N_GOTO_STMT);

    if (retval != NULL) {
        // Look for goto
        if (lookahead.type != T_GOTO) {
            syntax_error("goto", lookahead);
        }

        // Otherwise, consume it
        consume();

        // Look for identifier
        if (lookahead.type != T_IDENT) {
            syntax_error("identifier", lookahead);
        } else {
            memset(retval->data.goto_stmt.label, 0, MAX_LITERAL);
            snprintf(retval->data.goto_stmt.label, sizeof(retval->data.goto_stmt.label), "%s",
                     lookahead.literal);

            consume();
        }

        // Look for semicolon
        if (lookahead.type != T_SEMICOLON) {
            syntax_error(";", lookahead);
        }

        consume();
    }

    return retval;
}

// <var-decl> := ( 'struct' )? <type> <identifier> ( ':=' <expression> )? ';'
static node *parse_var_decl() {
    node *retval = mk_node(N_VAR_DECL);

    if (retval != NULL) {
        print_lookahead_debug("top of var_decl");

        // Look for the optional 'struct' keyword
        if (lookahead.type == T_STRUCT) {
            retval->data.var_decl.is_struct = true;
            // consume it
            consume();
        }

        // Get type
        if (retval->data.var_decl.is_struct) {
            // If we're a struct, our data type is 'struct' and our struct type is the identifier
            // after the 'struct' keyword
            retval->data.var_decl.type = D_STRUCT;
            memset(retval->data.var_decl.struct_type, 0, sizeof(retval->data.var_decl.struct_type));
            snprintf(retval->data.var_decl.struct_type, sizeof(retval->data.var_decl.struct_type),
                     "%s", lookahead.literal);
        } else {
            // Otherwise, we're a primitive data type
            retval->data.var_decl.type = keyword_to_type(lookahead.type);
        }

        // Consume the type declaration
        consume();

        // Look for the variable name
        if (lookahead.type != T_IDENT) {
            syntax_error("identifier name", lookahead);
        } else {
            memset(retval->data.var_decl.name, 0, MAX_LITERAL);
            snprintf(retval->data.var_decl.name, sizeof(retval->data.var_decl.name), "%s",
                     lookahead.literal);
        }

        consume();
        // Look for the assignment
        if (lookahead.type == T_ASSIGN) {

            // Consume assignment
            consume();
            retval->data.var_decl.value = parse_expression();

            if (lookahead.type != T_SEMICOLON) {
                syntax_error("; after expression", lookahead);
            }
            // Consume the semicolon
            consume();
        } else {
            // If we don't immediately assign a value, set a default based upon the type
            if (lookahead.type == T_SEMICOLON) {
                node *val_default = NULL;
                switch (retval->data.var_decl.type) {
                case D_INTEGER:
                    val_default                             = mk_node(N_INTEGER_LITERAL);
                    val_default->data.integer_literal.value = 0;
                    break;
                case D_FLOAT:
                    val_default                           = mk_node(N_FLOAT_LITERAL);
                    val_default->data.float_literal.value = 0.0;
                    break;
                case D_STRING:
                    val_default = mk_node(N_STRING_LITERAL);
                    // Empty string
                    memset(val_default->data.string_literal.value, 0,
                           sizeof(val_default->data.string_literal.value));
                    snprintf(val_default->data.string_literal.value,
                             sizeof(val_default->data.string_literal.value), "%s", "");
                    break;
                case D_BOOLEAN:
                    val_default                          = mk_node(N_BOOL_LITERAL);
                    val_default->data.bool_literal.value = 0; // false
                    strncpy(val_default->data.bool_literal.str_val, "false", sizeof("false"));
                    break;
                case D_STRUCT:
                    // For structs, don't assign a default value. We'll handle this at codegen time
                    // by allocating memory.
                    val_default = NULL;
                    break;
                default:
                    syntax_error("Unknown literal type", lookahead);
                }

                retval->data.var_decl.value = val_default;

                // Consume next token and we're done
                consume();
            } else {
                syntax_error("; after empty declaration", lookahead);
            }
        }
    }

    return retval;
}

static node *parse_member_decl() {
    node *retval = mk_node(N_MEMBER_DECL);

    if (retval != NULL) {
        switch (lookahead.type) {
        case T_INT:
        case T_BOOL:
        case T_STRING:
        case T_FLOAT:
            retval->data.member_decl.type = keyword_to_type(lookahead.type);
            break;
        default:
            syntax_error("type", lookahead);
        }

        consume();
        if (lookahead.type != T_IDENT) {
            syntax_error("identifier", lookahead);
        } else {
            memset(retval->data.member_decl.name, 0, MAX_LITERAL);
            snprintf(retval->data.member_decl.name, sizeof(retval->data.member_decl.name), "%s",
                     lookahead.literal);
        }

        consume();
        if (lookahead.type != T_SEMICOLON) {
            syntax_error(";", lookahead);
        }

        consume();
    }

    return retval;
}

// <struct-decl> := 'struct' <ident> 'then' <member-decls> 'end'
static node *parse_struct_decl() {
    node *retval = mk_node(N_STRUCT_DECL);

    if (retval != NULL) {
        retval->data.struct_decl.members = mk_vector();
        if (lookahead.type != T_STRUCT) {
            syntax_error("struct", lookahead);
        } else {
            retval->data.struct_decl.type = D_STRUCT;
        }

        consume();
        if (lookahead.type != T_IDENT) {
            syntax_error("identifier", lookahead);
        } else {
            memset(retval->data.struct_decl.name, 0, MAX_LITERAL);
            snprintf(retval->data.struct_decl.name, sizeof(retval->data.struct_decl.name), "%s",
                     lookahead.literal);
        }

        consume();
        if (lookahead.type != T_THEN) {
            syntax_error("then", lookahead);
        }

        // Parse member declarations
        consume();
        do {
            node *member = parse_member_decl();

            if (member != NULL) {
                vector_add(retval->data.struct_decl.members, member);
            } else {
                log_error("Unable to add NULL member decl to vector");
            }
        } while (lookahead.type != T_END);

        // Check for the 'end' token
        if (lookahead.type != T_END) {
            syntax_error("'end", lookahead);
        } else {
            // Consume 'end'
            consume();
        }
    }

    return retval;
}

// <struct-access-expr> := <ident> '.' <ident>
static node *parse_struct_access_expr() {
    node *retval = mk_node(N_STRUCT_ACCESS_EXPR);

    if (retval != NULL) {
        print_lookahead_debug("top of parse_struct_access");

        // Get struct name

        if (lookahead.type != T_IDENT) {
            syntax_error("identifier", lookahead);
        } else {
            memset(retval->data.struct_access.name, 0, sizeof(retval->data.struct_access.name));
            snprintf(retval->data.struct_access.name, sizeof(retval->data.struct_access.name), "%s",
                     lookahead.literal);

            // Consume struct name
            consume();
        }

        print_lookahead_debug("looking for dot");
        if (lookahead.type != T_DOT) {
            syntax_error(".", lookahead);
        } else {
            // Consume dot
            consume();
        }

        print_lookahead_debug("looking for ident");
        if (lookahead.type != T_IDENT) {
            syntax_error("member identifier", lookahead);
        } else {
            memset(retval->data.struct_access.member_name, 0,
                   sizeof(retval->data.struct_access.member_name));
            snprintf(retval->data.struct_access.member_name,
                     sizeof(retval->data.struct_access.member_name), "%s", lookahead.literal);

            // Consume member name
            consume();
        }
    }

    return retval;
}

// 'return' <expression> ';'
static node *parse_return_stmt() {
    node *retval = mk_node(N_RETURN_STMT);

    if (retval != NULL) {
        // Look for 'return'
        if (lookahead.type != T_RETURN) {
            syntax_error("return", lookahead);
        } else {
            consume();

            retval->data.return_stmt.expr = parse_expression();
            print_lookahead_debug("after return expr");

            //          consume();
            // Look for ;
            if (lookahead.type != T_SEMICOLON) {
                syntax_error("; after return expression", lookahead);
            }

            // Consume it
            consume();
        }
    }

    return retval;
}

static node *parse_identifier() {
    node *retval = mk_node(N_IDENT);
    print_lookahead_debug("parse_identifier");

    if (retval != NULL) {
        if (lookahead.type == T_IDENT) {
            // Assume the current lookahead is an identifier token
            memset(retval->data.identifier.name, 0, sizeof(retval->data.identifier.name));
            snprintf(retval->data.identifier.name, sizeof(retval->data.identifier.name), "%s",
                     lookahead.literal);

        } else {
            syntax_error("identifier", lookahead);
        }
    }

    return retval;
}
static node *parse_string_literal() {
    node *retval = mk_node(N_STRING_LITERAL);

    if (retval != NULL) {
        if (lookahead.type == L_STR) {
            print_lookahead_debug("inside parse_string_literal");
            retval->data.string_literal.type = D_STRING;
            memset(retval->data.string_literal.value, 0, sizeof(retval->data.string_literal.value));
            snprintf(retval->data.string_literal.value,
                     sizeof(retval->data.string_literal.value) - 1, "%s", lookahead.literal);
            // Size is MAX_LITERAL + 1, but we want to write the null terminator to the
            // MAX_LITERAL'th byte (ie. 0-1024)
            retval->data.string_literal.value[MAX_LITERAL] = '\0';
        } else {
            syntax_error("string literal", lookahead);
        }
    }

    return retval;
}
static node *parse_integer_literal() {
    node *retval = mk_node(N_INTEGER_LITERAL);

    if (retval != NULL) {
        retval->data.integer_literal.type  = D_INTEGER;
        retval->data.integer_literal.value = atoi(lookahead.literal);
    }

    return retval;
}

static node *parse_float_literal() {
    node *retval = mk_node(N_FLOAT_LITERAL);

    if (retval != NULL) {
        retval->data.float_literal.type  = D_FLOAT;
        retval->data.float_literal.value = atof(lookahead.literal);
    }

    return retval;
}
static node *parse_bool_literal() {
    node *retval = mk_node(N_BOOL_LITERAL);

    if (retval != NULL) {
        if (lookahead.type == T_TRUE || lookahead.type == T_FALSE) {
            retval->data.bool_literal.type = D_BOOLEAN;
            memset(retval->data.bool_literal.str_val, 0, sizeof(retval->data.bool_literal.str_val));
            snprintf(retval->data.bool_literal.str_val, sizeof(retval->data.bool_literal.str_val),
                     "%s", lookahead.literal);
            retval->data.bool_literal.value = (lookahead.type == T_TRUE) ? 1 : 0;
        } else {
            syntax_error("true or false", lookahead);
        }
    }
}
static node *parse_nil() {
    node *retval = mk_node(N_NIL);

    if (retval != NULL) {
        if (lookahead.type == T_NIL) {
            retval->data.nil.value = 0; // ALWAYS zero
        } else {
            syntax_error("nil", lookahead);
        }
    }

    return retval;
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
    case T_STRUCT:
        return D_STRUCT;
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
    case D_STRUCT:
        return "STRUCT";
        break;
    default:
        return "UNKNOWN";
        break;
    }
}

static const char *binop_to_str(token_type t) {
    switch (t) {
    case T_PLUS:
        return "+";
        break;
    case T_MINUS:
        return "-";
        break;
    case T_MUL:
        return "*";
        break;
    case T_DIV:
        return "/";
        break;
    case T_MOD:
        return "%";
        break;
    case T_AND:
        return "and";
        break;
    case T_OR:
        return "or";
        break;
    case T_EQ:
        return "==";
        break;
    case T_NE:
        return "!=";
        break;
    case T_GT:
        return ">";
        break;
    case T_GE:
        return ">=";
        break;
    case T_LT:
        return "<";
        break;
    case T_LE:
        return "<=";
        break;
    default:
        return "UNKNOWN";
        break;
    }
}

static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf(" ");
    }
}

static void print_node(node *n, int indent) {
    if (n == NULL) {
        log_error("Unable to access node for printing");
        exit(1);
    }

    print_indent(indent);
    switch (n->type) {
    case N_PROGRAM:
        printf("Program (\n");
        break;
    case N_BLOCK_STMT:
        printf("BlockStmt (\n");

        indent += INDENT_WIDTH;
        vecnode *bn = n->data.block_stmt.statements->head;
        while (bn != NULL) {
            node *n = bn->data;
            print_node(n, indent);

            bn = bn->next;
        }
        indent -= INDENT_WIDTH;

        print_indent(indent);
        printf("),\n");
        break;
    case N_VAR_DECL:
        printf("VarDecl (\n");
        print_indent(indent + INDENT_WIDTH);
        printf("Name: %s\n", n->data.var_decl.name);
        print_indent(indent + INDENT_WIDTH);
        printf("IsStruct: %s\n", (n->data.var_decl.is_struct ? "true" : "false"));
        print_indent(indent + INDENT_WIDTH);
        printf("Type: %s\n", type_to_str((data_type)n->data.var_decl.type));
        print_indent(indent + INDENT_WIDTH);
        printf("StructType: %s\n", n->data.var_decl.struct_type);
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
    case N_LABEL_DECL:
        printf("LabelDecl (\n");
        print_indent(indent + INDENT_WIDTH);
        printf("Name: %s\n", n->data.label_decl.name);
        print_indent(indent);
        printf("), \n");
        break;
    case N_GOTO_STMT:
        printf("GotoStmt (\n");
        print_indent(indent + INDENT_WIDTH);
        printf("Label: %s\n", n->data.goto_stmt.label);
        print_indent(indent);
        printf("), \n");
        break;
    case N_FUNC_DECL:
        printf("FuncDecl (\n");
        print_indent(indent + INDENT_WIDTH);
        printf("Name: %s\n", n->data.function_decl.name);
        print_indent(indent + INDENT_WIDTH);
        printf("Type: %s\n", type_to_str((data_type)n->data.function_decl.type));
        print_indent(indent + INDENT_WIDTH);
        printf("Formals ( ");
        indent += INDENT_WIDTH;
        if (n->data.function_decl.formals == NULL) {
            printf("None )\n");
        } else {
            printf("\n");
            vecnode *fn = n->data.function_decl.formals->head;

            while (fn != NULL) {
                node *n = fn->data;
                print_node(n, indent + INDENT_WIDTH);
                fn = fn->next;
            }

            print_indent(indent);
            printf(")\n");
        }
        // Print FuncDecl body
        print_indent(indent);
        printf("Body:\n");

        print_node(n->data.function_decl.body, indent + INDENT_WIDTH);
        indent -= INDENT_WIDTH;

        print_indent(indent);
        printf("),\n");
        break;
    case N_RETURN_STMT:
        printf("ReturnStmt (\n");
        print_indent(indent + INDENT_WIDTH);
        printf("Expression: \n");
        indent += INDENT_WIDTH;
        print_node(n->data.return_stmt.expr, indent + INDENT_WIDTH);
        indent -= INDENT_WIDTH;
        print_indent(indent);
        printf("),\n");
        break;
    case N_CALL_EXPR:
        printf("CallExpr (\n");
        print_indent(indent + INDENT_WIDTH);
        printf("Function name: %s\n", n->data.call_expr.func_name);
        print_indent(indent + INDENT_WIDTH);
        printf("Args: ");
        if (n->data.call_expr.args == NULL) {
            printf("None\n");
        } else {
            indent += INDENT_WIDTH;
            printf("\n");
            vecnode *an = n->data.call_expr.args->head;

            while (an != NULL) {
                node *n = an->data;
                print_node(n, indent + INDENT_WIDTH);
                an = an->next;
            }
            indent -= INDENT_WIDTH;
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
        vecnode *m = n->data.struct_decl.members->head;
        if (m != NULL) {
            printf("\n");
            while (m != NULL) {
                print_node(m->data, indent + INDENT_WIDTH);
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
    case N_INTEGER_LITERAL:
        printf("IntegerLiteral (\n");
        print_indent(indent + INDENT_WIDTH);
        printf("Type: %s\n", type_to_str((data_type)n->data.integer_literal.type));
        print_indent(indent + INDENT_WIDTH);
        printf("Value: %d\n", n->data.integer_literal.value);
        print_indent(indent);
        printf(")\n");
        break;
    case N_FLOAT_LITERAL:
        printf("FloatLiteral (\n");
        print_indent(indent + INDENT_WIDTH);
        printf("Type: %s\n", type_to_str((data_type)n->data.float_literal.type));
        print_indent(indent + INDENT_WIDTH);
        printf("Value: %f\n", n->data.float_literal.value);
        print_indent(indent);
        printf(")\n");
        break;
    case N_STRING_LITERAL:
        printf("StringLiteral (\n");
        print_indent(indent + INDENT_WIDTH);
        printf("Type: %s\n", type_to_str((data_type)n->data.string_literal.type));
        print_indent(indent + INDENT_WIDTH);
        printf("Value: %s\n", n->data.string_literal.value);
        print_indent(indent);
        printf(")\n");
        break;
    case N_BOOL_LITERAL:
        printf("BoolLiteral (\n");
        print_indent(indent + INDENT_WIDTH);
        printf("Type: %s\n", type_to_str((data_type)n->data.bool_literal.type));
        print_indent(indent + INDENT_WIDTH);
        printf("Value: %d\n", n->data.bool_literal.value);
        print_indent(indent + INDENT_WIDTH);
        printf("StringValue: %s\n", n->data.bool_literal.str_val);
        print_indent(indent);
        printf(")\n");
        break;
    case N_NIL:
        printf("Nil (\n");
        print_indent(indent + INDENT_WIDTH);
        printf("Value: %d\n", n->data.nil.value);
        print_indent(indent);
        printf(")\n");
        break;
    case N_IDENT:
        printf("Identifier (\n");
        print_indent(indent + INDENT_WIDTH);
        printf("Name: %s\n", n->data.identifier.name);
        print_indent(indent);
        printf("),\n");
        break;
    case N_IF_STMT:
        printf("IfStmt (\n");
        print_indent(indent + INDENT_WIDTH);
        printf("Test: \n");

        indent += INDENT_WIDTH;
        print_node(n->data.if_stmt.test, indent + INDENT_WIDTH);
        indent -= INDENT_WIDTH;

        print_indent(indent + INDENT_WIDTH);
        printf("Body: \n");

        indent += INDENT_WIDTH;
        print_node(n->data.if_stmt.body, indent + INDENT_WIDTH);
        indent -= INDENT_WIDTH;

        print_indent(indent + INDENT_WIDTH);
        printf("Else: ");

        if (n->data.if_stmt.else_stmt == NULL) {
            printf("None \n");
        } else {
            printf("\n");
            indent += INDENT_WIDTH;
            print_node(n->data.if_stmt.else_stmt, indent + INDENT_WIDTH);
            indent -= INDENT_WIDTH;
        }

        print_indent(indent);
        printf("),\n");
        break;
    case N_WHILE_STMT:
        printf("WhileStmt (\n");
        print_indent(indent + INDENT_WIDTH);
        printf("Test: \n");

        indent += INDENT_WIDTH;
        print_node(n->data.while_stmt.test, indent + INDENT_WIDTH);
        indent -= INDENT_WIDTH;

        print_indent(indent + INDENT_WIDTH);
        printf("Body: \n");

        indent += INDENT_WIDTH;
        print_node(n->data.while_stmt.body, indent + INDENT_WIDTH);
        indent -= INDENT_WIDTH;

        print_indent(indent);
        printf("),\n");
        break;
    case N_EMPTY_EXPR:
        printf("EmptyExpr (),\n");
        break;
    case N_NEG_EXPR:
        printf("NegExpr (\n");
        print_indent(indent + INDENT_WIDTH);
        printf("Expr: \n");

        indent += INDENT_WIDTH;
        print_node(n->data.neg_expr.expr, indent + INDENT_WIDTH);
        indent -= INDENT_WIDTH;

        print_indent(indent);
        printf(")\n");
        break;
    case N_NOT_EXPR:
        printf("NotExpr (\n");
        print_indent(indent + INDENT_WIDTH);
        printf("Expr: \n");

        indent += INDENT_WIDTH;
        print_node(n->data.not_expr.expr, indent + INDENT_WIDTH);
        indent -= INDENT_WIDTH;

        print_indent(indent);
        printf(")\n");
        break;
    case N_BINOP_EXPR:
        printf("BinOpExpr (\n");
        print_indent(indent + INDENT_WIDTH);
        printf("LHS: \n");

        indent += INDENT_WIDTH;
        print_node(n->data.bin_op_expr.lhs, indent + INDENT_WIDTH);
        indent -= INDENT_WIDTH;

        print_indent(indent + INDENT_WIDTH);
        printf("Operator: %s\n", binop_to_str(n->data.bin_op_expr.operator));

        print_indent(indent + INDENT_WIDTH);
        printf("RHS: \n");
        indent += INDENT_WIDTH;
        print_node(n->data.bin_op_expr.rhs, indent + INDENT_WIDTH);
        indent -= INDENT_WIDTH;

        print_indent(indent);
        printf("),\n");
        break;
    case N_ASSIGN_EXPR:
        printf("AssignExpr (\n");
        print_indent(indent + INDENT_WIDTH);
        printf("LHS: \n");

        indent += INDENT_WIDTH;
        print_node(n->data.assign_expr.lhs, indent + INDENT_WIDTH);
        indent -= INDENT_WIDTH;

        print_indent(indent + INDENT_WIDTH);
        printf("RHS: \n");
        indent += INDENT_WIDTH;
        print_node(n->data.assign_expr.rhs, indent + INDENT_WIDTH);
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
        print_indent(indent);
        printf("Statements (");
        if (ast->data.program.statements->head != NULL) {
            printf("\n");
            indent += INDENT_WIDTH;
            vecnode *vn = ast->data.program.statements->head;
            while (vn != NULL) {
                node *n = vn->data;
                print_node(n, indent);

                vn = vn->next;
            }

            indent -= INDENT_WIDTH;
            print_indent(indent);
        } else {
            printf("None");
        }
        printf(")\n");
    }

    indent -= INDENT_WIDTH;
    printf(")\n");
    return;
}
