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

// Globals
static symtab_t *symbol_table = NULL;
static bool created_table     = false;

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
            snprintf(name, strlen(name), "%s", n->data.var_decl.name);
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
        printf("here\n");
        if (!is_duplicate(n->data.function_decl.name)) {
            binding_t *b = mk_binding();

            if (b != NULL) {
                snprintf(b->name, sizeof(b->name), "%s", n->data.function_decl.name);
                b->data_type   = SYM_DTYPE_UNKNOWN;
                b->object_type = SYM_OTYPE_FUNCTION;

                ht_insert(symbol_table->table, b->name, b);
            }

            if (vector_length(n->data.function_decl.formals) > 0) {
                vector *vec = n->data.function_decl.formals;

                vecnode *vn = vec->head;

                while (vn != NULL) {
                    typecheck(vn->data);

                    vn = vn->next;
                }
            }
        }
    }
}

static void typecheck_formal(node *n) {
    if (n != NULL) {
        if (!is_duplicate(n->data.formal.name)) {
            binding_t *b = mk_binding();

            if (b != NULL) {
                snprintf(b->name, sizeof(b->name), "%s", n->data.formal.name);
                switch (n->data.formal.type) {
                case D_INTEGER:
                    b->data_type = SYM_DTYPE_INTEGER;
                    break;
                case D_FLOAT:
                    b->data_type = SYM_DTYPE_FLOAT;
                    break;
                case D_STRING:
                    b->data_type = SYM_DTYPE_STRING;
                    break;
                case D_BOOLEAN:
                    b->data_type = SYM_DTYPE_BOOLEAN;
                    break;
                case D_VOID:
                    b->data_type = SYM_DTYPE_VOID;
                    break;
                case D_NIL:
                default:
                    b->data_type = SYM_DTYPE_UNKNOWN;
                }
                b->object_type = SYM_OTYPE_VARIABLE;

                ht_insert(symbol_table->table, b->name, b);
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
        if (!created_table) {
            symbol_table = symtab_new();

            if (symbol_table == NULL) {
                log_error("Unable to create symbol table");
            } else {
                printf("Successfully allocated symbol table\n");
                created_table = true;
            }
        }

        switch (ast->type) {
        case N_PROGRAM:
            typecheck_program(ast);
            break;
        case N_BLOCK_STMT:
            typecheck_block_stmt(ast);
            break;
        case N_VAR_DECL:
            //            typecheck_var_decl(ast);
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
            typecheck_formal(ast);
            break;
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
    if (symbol_table != NULL) {
        // Search hashtable for symbols matching ident
        hashtable *ht = symbol_table->table;
        binding_t *b  = ht_lookup(ht, (char *)ident, ht_compare_binding);

        if (b != NULL) {
            printf("Found an existing binding!\n");
            print_binding(b);
            retval = true;
        }
    }

    return retval;
}

// Might not be needed
void print_checked_ast(node *ast) { return; }

void print_symbol_tables() {
    hashtable *ht = symbol_table->table;

    if (ht == NULL) {
        log_error("Unable to access symbol table for printing!");
    }

    // Iterate over hash table and print each key/value pair
    printf("Symbol Table\n");
    printf("============\n");
    for (int idx = 0; idx < MAX_SLOTS; idx++) {
        if (vector_length(ht->slots[idx]) <= 0) {
            continue;
        } else {
            vector *vec = ht->slots[idx];

            vecnode *vn = vec->head;

            while (vn != NULL) {
                binding_t *b = (binding_t *)vn->data;
                print_binding(b);

                vn = vn->next;
            }
        }
    }
}
