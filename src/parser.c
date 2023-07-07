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

// Grammar productions
static vector *parse_statements(void);
static node *parse_statement(bool *more);
static node *parse_block_stmt(void);
static node *parse_for_stmt(void);
static node *parse_while_stmt(void);
static node *parse_if_stmt(void);
static node *parse_ifelse_stmt(void);
static node *parse_assign_expr(void);
static node *parse_function_call_stmt(void);
static node *parse_expression(void);
static node *parse_function_decl(void);
static node *parse_label_decl(void);
static node *parse_var_decl(void);
static node *parse_struct_decl(void);
static node *parse_return_stmt(void);

// Precedence rules
static node *parse_add_expr(void);
static node *parse_mult_expr(void);
static node *parse_primary_expr(void);

static node *parse_identifier(void);
static node *parse_string_literal(void);
static node *parse_integer_literal(void);
static node *parse_float_literal(void);
static node *parse_bool_literal(void);
static node *parse_nil(void);

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

static void print_lookahead_debug(const char *msg) {
    if (strlen(msg) > 0) {
        printf("Msg: %s\n", msg);
    }
    printf("Lookahead type: %d\n", lookahead.type);
    printf("Lookahead literal: %s\n", lookahead.literal);
    printf("Line: %d\n", lookahead.line);
    printf("Column: %d\n", lookahead.col);
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
                vector_add(retval, new_node);
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
//              | <struct-decl
//              | <expression>
//              | ( <expression> )
static node *parse_statement(bool *more) {
    node *retval;
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
    case N_IFELSE_STMT:
        retval = parse_ifelse_stmt();
        break;
    case T_FUNC:
        retval = parse_function_decl();
        break;
    case T_IDENT: {
        print_lookahead_debug("ident");
        token *tmp = peek();
        printf("tmp literal: %s\n", tmp->literal);

        if (strcmp(tmp->literal, ":=") == 0) {
            consume();
            retval = parse_expression();
            break;
        } else if (strcmp(tmp->literal, ":") == 0) {
            retval = parse_label_decl();
            break;
        } else if (strcmp(tmp->literal, "(") == 0) {
            retval = parse_function_call_stmt();
            break;
        } else if ((strcmp(tmp->literal, "+") == 0) || (strcmp(tmp->literal, "-") == 0) ||
                   (strcmp(tmp->literal, "*") == 0) || (strcmp(tmp->literal, "/") == 0) ||
                   (strcmp(tmp->literal, "%") == 0) || (strcmp(tmp->literal, ">") == 0) ||
                   (strcmp(tmp->literal, "<") == 0) || (strcmp(tmp->literal, ">=") == 0) ||
                   (strcmp(tmp->literal, "<=") == 0) || (strcmp(tmp->literal, "==") == 0) ||
                   (strcmp(tmp->literal, "!=") == 0)) {
            retval = parse_expression(); // binop exprs when dealing with variables
            break;
        } else {
            printf("Parser Error: Unknown case when encountering N_IDENT\n");
            break;
        }
    }
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
    case T_STRUCT:
        retval = parse_struct_decl();
        break;
    // case N_EXPR:
    //     retval = parse_expression();
    //     break;
    case T_LPAREN:
        retval = parse_expression();
        break;
    case T_RETURN:
        retval = parse_return_stmt();
    case T_END: // This should be the end of most bodies (conditionals, functions, etc)
    default:
        *more = false;
    }

    return retval;
}

static node *parse_block_stmt() { assert(0 && "Not yet implemented"); }
static node *parse_for_stmt() { assert(0 && "Not yet implemented"); }
static node *parse_while_stmt() { assert(0 && "Not yet implemented"); }
static node *parse_if_stmt() { assert(0 && "Not yet implemented"); }
static node *parse_ifelse_stmt() { assert(0 && "Not yet implemented"); }

// Addition and subtraction
static node *parse_add_expr() {
    print_lookahead_debug("begin add_expr");
    consume();
    node *lhs = parse_mult_expr();
    //    consume();

    node *rhs;

    print_lookahead_debug("before +/- selection");
    node *new_binop_add = mk_node(N_BINOP_EXPR);
    switch (lookahead.type) {
    case T_PLUS:
    case T_MINUS:
        new_binop_add->data.bin_op_expr.operator= lookahead.type;
        break;
        // syntax_error("+ or -", lookahead);
    }
    // Consume operator
    consume();

    rhs = parse_mult_expr();
    //    consume();

    new_binop_add->data.bin_op_expr.lhs = lhs;
    new_binop_add->data.bin_op_expr.rhs = rhs;

    print_lookahead_debug("end mult_expr");

    return new_binop_add;
}

// Multiplication, division, and modulus
static node *parse_mult_expr() {
    print_lookahead_debug("begin mult_expr");
    //    consume();

    // literal or identifier
    node *lhs = parse_primary_expr();
    //    consume();

    node *rhs;

    node *new_binop_primary = mk_node(N_BINOP_EXPR);
    switch (lookahead.type) {
    case T_MUL:
    case T_DIV:
    case T_MOD:
        new_binop_primary->data.bin_op_expr.operator= lookahead.type;
        break;
        // syntax_error("* or / or %", lookahead);
    }
    // Consume operator
    consume();

    rhs = parse_primary_expr();
    //    consume();

    new_binop_primary->data.bin_op_expr.lhs = lhs;
    new_binop_primary->data.bin_op_expr.rhs = rhs;
    print_lookahead_debug("end mult_expr");

    return new_binop_primary;
}

// Literals and grouping expressions
static node *parse_primary_expr() {
    node *retval;
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
    case T_IDENT:
        retval = parse_identifier();
        break;
    case T_NIL:
        retval = parse_nil();
        break;
    }

    // Consume the literal or ident
    //    consume();
    print_lookahead_debug("end primary_expr");

    return retval;
}

// <assign-expr> := <ident> ( '[' <expression> ']' )? ':=' <expression> ';'
static node *parse_assign_expr() {
    node *retval = mk_node(N_ASSIGN_EXPR);

    if (retval != NULL) {
        retval->data.assign_expr.lhs = parse_add_expr();

        if (lookahead.type == T_ASSIGN) {
            consume();
            retval->data.assign_expr.rhs = parse_assign_expr();
        }
    }

    return retval;
}

static node *parse_function_call_stmt() { assert(0 && "Not yet implemented"); }
static node *parse_function_decl() { assert(0 && "Not yet implemented"); }

static node *parse_expression() {
    struct node *retval;

    switch (lookahead.type) {
    case T_ASSIGN:
        retval = parse_assign_expr();
        break;
    default:
        assert(0 && "Expression type not yet implemented");
    }

    return retval;
}
static node *parse_label_decl() { assert(0 && "Not yet implemented"); }
static node *parse_var_decl() {
    node *retval = mk_node(N_VAR_DECL);

    if (retval != NULL) {
        retval->data.var_decl.type = keyword_to_type(lookahead.type);

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
            retval->data.var_decl.value = parse_expression();
        }

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
            default:
                syntax_error("Unknown literal type", lookahead);
            }

            retval->data.var_decl.value = val_default;

            // Consume next token and we're done
            consume();
        } else {
            syntax_error(";", lookahead);
        }
    }

    return retval;
}

static node *parse_struct_decl() { assert(0 && "Not yet implemented"); }
static node *parse_return_stmt() { assert(0 && "Not yet implemented"); }
static node *parse_identifier() {
    node *retval = mk_node(N_IDENT);

    if (retval != NULL) {
        if (lookahead.type == T_IDENT) {
            // Assume the current lookahead is an identifier token
            memset(retval->data.identifier.name, 0, sizeof(retval->data.identifier.name));
            snprintf(retval->data.identifier.name, sizeof(retval->data.identifier.name), "%s",
                     lookahead.literal);

            consume();
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
        retval->data.integer_literal.value = atoi(lookahead.literal);
    }

    return retval;
}

static node *parse_float_literal() {
    node *retval = mk_node(N_FLOAT_LITERAL);

    if (retval != NULL) {
        retval->data.float_literal.value = atof(lookahead.literal);
    }

    return retval;
}
static node *parse_bool_literal() {
    node *retval = mk_node(N_BOOL_LITERAL);

    if (retval != NULL) {
        if (lookahead.type == T_TRUE || lookahead.type == T_FALSE) {
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

            consume();
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
        printf("Formals {\n");

        indent += INDENT_WIDTH;
        vecnode *f = n->data.function_decl.formals->head;
        if (f != NULL) {
            printf("\n");
            while (f != NULL) {
                print_node(f->data, indent + INDENT_WIDTH);
                f = f->next;
            }
        } else {
            printf("None\n");
        }

        print_indent(indent);
        printf("}\n");

        // Print FuncDecl body
        print_indent(indent);
        printf("Body {\n");

        vecnode *vn = n->data.function_decl.body->data.block_stmt.statements->head;
        while (vn->next != NULL) {
            print_node(vn->data, indent + INDENT_WIDTH);

            vn = vn->next;
        }

        print_indent(indent);
        printf("}\n");

        /*
                print_indent(indent);
                printf("Return:\n");

                if (n->data.function_decl.return_expr == NULL) {
                    print_indent(indent + INDENT_WIDTH);
                    printf("None\n");
                } else {
                    print_node(n->data.function_decl.return_expr, indent + INDENT_WIDTH);
                }
                indent -= INDENT_WIDTH;

                print_indent(indent);
        */
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
        printf("Value: %d\n", n->data.integer_literal.value);
        print_indent(indent);
        printf(")\n");
        break;
    case N_FLOAT_LITERAL:
        printf("FloatLiteral (\n");
        print_indent(indent + INDENT_WIDTH);
        printf("Value: %f\n", n->data.float_literal.value);
        print_indent(indent);
        printf(")\n");
        break;
    case N_STRING_LITERAL:
        printf("StringLiteral (\n");
        print_indent(indent + INDENT_WIDTH);
        printf("Value: %s\n", n->data.string_literal.value);
        print_indent(indent);
        printf(")\n");
        break;
    case N_BOOL_LITERAL:
        printf("BoolLiteral (\n");
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
    case N_EMPTY_EXPR:
        printf("EmptyExpr (),\n");
        break;
    case N_BINOP_EXPR:
        printf("BinOpExpr (\n");
        print_indent(indent + INDENT_WIDTH);
        printf("LHS: \n");

        indent += INDENT_WIDTH;
        print_node(n->data.bin_op_expr.lhs, indent + INDENT_WIDTH);
        indent -= INDENT_WIDTH;

        printf("RHS: \n");
        indent += INDENT_WIDTH;
        print_node(n->data.bin_op_expr.rhs, indent + INDENT_WIDTH);
        indent -= INDENT_WIDTH;

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
        printf("Statements {\n");
        indent += INDENT_WIDTH;
        vecnode *vn = ast->data.program.statements->head;
        while (vn->next != NULL) {
            node *n = vn->data;
            print_node(n, indent);

            vn = vn->next;
        }

        indent -= INDENT_WIDTH;
        print_indent(indent);
        printf("}\n");
    }

    indent -= INDENT_WIDTH;
    printf(")\n");
    return;
}
