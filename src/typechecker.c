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
    exit(1);
}

// Gets the type of a node that contains a type
static type_t get_type(node *n) {
    type_t retval;
    memset(&retval, 0, sizeof(type_t));
    snprintf(retval.struct_type, strlen(retval.struct_type), "NONE");

    if (n != NULL) {
        switch (n->type) {
        case N_VAR_DECL:
            retval.datatype = n->data.var_decl.type;
            snprintf(retval.struct_type, strlen(retval.struct_type), "%s",
                     n->data.var_decl.struct_type);
            break;
        case N_FUNC_DECL:
            retval.datatype = n->data.function_decl.type;
            snprintf(retval.struct_type, strlen(retval.struct_type), "%s",
                     n->data.function_decl.struct_type);
            break;
        case N_STRUCT_DECL:
            retval.datatype = n->data.struct_decl.type;
            snprintf(retval.struct_type, strlen(retval.struct_type), "%s",
                     n->data.struct_decl.name);
            break;
        case N_FORMAL:
            retval.datatype = n->data.formal.type;
            snprintf(retval.struct_type, strlen(retval.struct_type), "%s",
                     n->data.formal.struct_type);
            break;
        case N_MEMBER_DECL:
            retval.datatype = n->data.member_decl.type;
            break;
        case N_INTEGER_LITERAL:
            retval.datatype = n->data.integer_literal.type;
            break;
        case N_FLOAT_LITERAL:
            retval.datatype = n->data.float_literal.type;
            break;
        case N_STRING_LITERAL:
            retval.datatype = n->data.string_literal.type;
            break;
        case N_BOOL_LITERAL:
            retval.datatype = n->data.bool_literal.type;
            break;
        case N_NIL:
            retval.datatype = D_NIL;
            break;
        default:
            type_error("Requested node does not contain a coercible type", n);
            break;
        }

    } else {
        log_error("Unable to access node for type retrieval");
    }

    return retval;
}

static bool coerce_to(node *a, node *b) {
    bool retval = false;
    if (a != NULL) {
        if (b != NULL) {
            type_t a_type = get_type(a);
            type_t b_type = get_type(b);

            // Do the datatypes match?
            if (a_type.datatype == b_type.datatype) {
                // Are the struct types both NONE?
                if ((strcmp(a_type.struct_type, "NONE") == 0) &&
                    (strcmp(b_type.struct_type, "NONE") == 0)) {
                    // If yes, we can say both types are compatible.
                    retval = true;
                } else {
                    if (strcmp(a_type.struct_type, b_type.struct_type) == 0) {
                        retval = true;
                    }
                }
            }
        } else {
            log_error("Unable to access Node B for type coercion");
        }
    } else {
        log_error("Unable to access Node A for type coercion");
    }

    return retval;
}

static data_type get_literal_type(node *n) {
    if (n != NULL) {
        switch (n->type) {
        case N_INTEGER_LITERAL:
            return D_INTEGER;
        case N_FLOAT_LITERAL:
            return D_FLOAT;
        case N_STRING_LITERAL:
            return D_STRING;
        case N_BOOL_LITERAL:
            return D_BOOLEAN;
        case N_NIL:
            return D_NIL;
        default:
            type_error("Unable to resolve data type from literal", n);
        }
    }

    return D_UNKNOWN;
}

static void typecheck_program(node *n) {
    printf("Typechecking program\n");
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
    printf("Typechecking block statement\n");
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
    printf("Typechecking var decl\n");
    if (n != NULL) {
        if (!is_duplicate(n->data.var_decl.name)) {
            if (!coerce_to(n, n->data.var_decl.value)) {
                type_error("Mismatch between variable declaration and immediate value", n);
            }

            binding_t *b = mk_binding();
            if (b != NULL) {
                snprintf(b->name, sizeof(b->name), "%s", n->data.var_decl.name);
                snprintf(b->struct_type, sizeof(b->struct_type), "%s",
                         n->data.var_decl.struct_type);
                b->data_type   = ast_data_type_to_binding_data_type(n->data.var_decl.type);
                b->object_type = SYM_OTYPE_VARIABLE;
                b->is_array    = n->data.var_decl.is_array;
                b->array_dims  = n->data.var_decl.num_dimensions;

                ht_insert(symbol_table->table, b->name, b);
            }
        } else {
            char msg[MAX_ERROR_LEN] = {0};
            snprintf(msg, sizeof(msg), "Identifier already defined in this scope: %s",
                     n->data.var_decl.name);
            type_error(msg, n);
        }
    }
}

static void typecheck_func_decl(node *n) {
    printf("Typechecking function decl\n");
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

            typecheck(n->data.function_decl.body);
        }
    }
}

static void typecheck_formal(node *n) {
    printf("Typechecking formal\n");
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
                b->is_array    = n->data.formal.is_array;
                b->array_dims  = n->data.formal.num_dimensions;

                ht_insert(symbol_table->table, b->name, b);
            }
        }
    }
}

static void typecheck_assign_expr(node *n) {
    printf("Typechecking assign expr\n");
    if (n != NULL) {
        // First, check if the LHS is in the symbol table
        typecheck(n->data.assign_expr.lhs);

        // Then, the RHS
        //        typecheck(n->data.assign_expr.lhs);

        if (!coerce_to(n->data.assign_expr.lhs, n->data.assign_expr.rhs)) {
            type_error("lol what", n);
        }
    }
}

static void typecheck_ident(node *n) {
    printf("Typechecking ident\n");
    if (n != NULL) {
        if (is_duplicate(n->data.identifier.name)) {
            char msg[MAX_ERROR_LEN] = {0};
            snprintf(msg, sizeof(msg), "Identifier already defined in this scope: %s",
                     n->data.var_decl.name);
            type_error(msg, n);
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
                symbol_table->level = 0;
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
            typecheck_ident(ast);
            break;
        case N_IF_STMT:
        case N_WHILE_STMT:
        case N_EMPTY_EXPR:
        case N_NEG_EXPR:
        case N_NOT_EXPR:
        case N_BINOP_EXPR:
        case N_ASSIGN_EXPR:
            typecheck_assign_expr(ast);
            break;
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
    printf("Symbol Table: Level %d\n", symbol_table->level);
    printf("=========================\n");
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
