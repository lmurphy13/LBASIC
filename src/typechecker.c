/**
 * LBASIC Semantic Analyzer Module
 * File: typechecker.c
 * Author: Liam M. Murphy
 */

#include "typechecker.h"

#include "ast.h"
#include "error.h"
#include "symtab.h"

#include <stdio.h>
#include <string.h>

// Temporary
int indent     = 0;
vector *idents = NULL;

// Prototypes
static bool is_duplicate(const char *ident);

static void type_error(const char *str, node *n) {
    printf("Type Error! Node type: %d\n", n->type);
    printf("%s\n", str);
    print_node(n, INDENT_WIDTH);
}

static void typecheck_program(node *n) {
    if (n != NULL) {
        // Typecheck children
        if (n->data.program.statements->head != NULL) {
            vecnode *vn = n->data.program.statements->head;
            while (vn != NULL) {
                node *n = vn->data;

                typecheck(n);

                vn = vn->next;
            }
        }
    }
}

static void typecheck_block_stmt(node *n) {
    if (n != NULL) {
        vecnode *bn = n->data.block_stmt.statements->head;
        while (bn != NULL) {
            node *n = bn->data;
            typecheck(n);

            bn = bn->next;
        }
    }
}

static void typecheck_var_decl(node *n) {
    if (n != NULL) {
        /*
                printf("VarDecl (\n");
                printf("Name: %s\n", n->data.var_decl.name);
                printf("IsStruct: %s\n", (n->data.var_decl.is_struct ? "true" : "false"));
                printf("IsArray: %s\n", (n->data.var_decl.is_array ? "true" : "false"));
                printf("Dimensions: %d\n", n->data.var_decl.num_dimensions);
                printf("Type: %s\n", type_to_str((data_type)n->data.var_decl.type));
                printf("StructType: %s\n", n->data.var_decl.struct_type);
                printf("Value: ");
        */
        if (!is_duplicate(n->data.var_decl.name)) {
            char *name = (char *)malloc(strlen(n->data.var_decl.name));
            snprintf(name, sizeof(name), "%s", n->data.var_decl.name);
            vector_add(idents, name);
        } else {
            char msg[MAX_ERROR_LEN] = {0};
            snprintf(msg, sizeof(msg), "Identifier already defined in this scope: %s",
                     n->data.var_decl.name);
            type_error(msg, n);
        }
    }
}

static void typecheck_func_decl(node *n) {
    if (n != NULL) {
        if (!is_duplicate(n->data.function_decl.name)) {
            binding_t *b = mk_binding();

            if (b != NULL) {
                snprintf(b->name, sizeof(b->name), "%s", n->data.function_decl.name);
                vector_add(idents, b);
            }
        }
    }
}

/*
 * Pass 1: Build a symbol table
 * Pass 2: Verify types and scope
 */
void typecheck(node *ast) {
    if (ast != NULL) {
        idents = mk_vector();

        switch (ast->type) {
        case N_PROGRAM:
            typecheck_program(ast);
            break;
        case N_BLOCK_STMT:
            typecheck_block_stmt(ast);
            break;
        case N_VAR_DECL:
            typecheck_var_decl(ast);
            break;
        case N_LABEL_DECL:
        case N_GOTO_STMT:
        case N_FUNC_DECL:
            typecheck_func_decl(ast);
            break;
        case N_RETURN_STMT:
        case N_CALL_EXPR:
        case N_STRUCT_DECL:
        case N_STRUCT_ACCESS_EXPR:
        case N_ARRAY_INIT_EXPR:
        case N_ARRAY_ACCESS_EXPR:
        case N_FORMAL:
        case N_MEMBER_DECL:
        case N_LITERAL:
        case N_INTEGER_LITERAL:
        case N_FLOAT_LITERAL:
        case N_STRING_LITERAL:
        case N_BOOL_LITERAL:
        case N_NIL:
        case N_IDENT:
        case N_IF_STMT:
        case N_WHILE_STMT:
        case N_EMPTY_EXPR:
        case N_NEG_EXPR:
        case N_NOT_EXPR:
        case N_BINOP_EXPR:
        case N_ASSIGN_EXPR:
        default:
            type_error("Unknown node type", ast);
            break;
        }
    } else {
        log_error("Unable to access AST for type checking");
    }
}

// Prototype definitions

static bool is_duplicate(const char *ident) {
    bool retval = false;

    if (ident == NULL) {
        log_error("Unable to access identifier before symbol table lookup");
    }

    // Eventually a symbol table
    if (idents != NULL) {
        printf("count: %d\n", vector_length(idents));
        // Search vector for symbols matching ident
        if (idents->count > 0) {
            vecnode *vn = idents->head;

            while (vn != NULL) {
                char *str = (char *)vn->data;
                printf("str: %s\n", str);

                if (strcmp(ident, str) == 0) {
                    retval = true;
                    break;
                }

                vn = vn->next;
            }
        }
    }

    return retval;
}

// Might not be needed
void print_checked_ast(node *ast) { return; }
