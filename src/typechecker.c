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

#define N_BUILTINS 2

typedef struct builtin_s {
    char name[MAX_LITERAL];
    data_type type;
} builtin_t;

// Globals
static symtab_t *symbol_table = NULL;
static bool created_table     = false;

// Prototypes
static bool is_duplicate(const char *ident);
static bool is_builtin(const char *ident, data_type *type);

static void type_error(const char *str, node *n) {
    printf("Type Error! Node type: %d\n", n->type);
    printf("%s\n", str);
    print_node(n, INDENT_WIDTH);
    exit(1);
}

static type_t query_symbol_table(const char *ident) {
    type_t retval;
    memset(&retval, 0, sizeof(type_t));
    retval.datatype = D_UNKNOWN;
    snprintf(retval.struct_type, strlen(retval.struct_type), "NONE");

    if (ident != NULL) {
        if (symbol_table != NULL) {
            // Search hashtable for symbols matching ident
            hashtable *ht = symbol_table->table;
            binding_t *b  = ht_lookup(ht, (char *)ident, ht_compare_binding);

            if (b != NULL) {
                retval.datatype = sym_data_to_data_type(b->data_type);
                if (strlen(b->struct_type) > 0) {
                    snprintf(retval.struct_type, strlen(retval.struct_type), "%s", b->struct_type);
                }
            }
        }
    }

    return retval;
}

// Gets the type of a node that contains a type
// These should be "leaf" nodes
static bool get_type(node *n, type_t *out) {
    bool retval = true;

    memset(out, 0, sizeof(type_t));
    snprintf(out->struct_type, strlen(out->struct_type), "NONE");

    if (n != NULL) {
        printf("Getting the type of\n");
        print_node(n, INDENT_WIDTH);

        switch (n->type) {
            case N_VAR_DECL:
                out->datatype = n->data.var_decl.type;
                snprintf(out->struct_type, strlen(out->struct_type), "%s",
                         n->data.var_decl.struct_type);
                break;
            case N_FUNC_DECL:
                out->datatype = n->data.function_decl.type;
                snprintf(out->struct_type, strlen(out->struct_type), "%s",
                         n->data.function_decl.struct_type);
                break;
            case N_STRUCT_DECL:
                out->datatype = n->data.struct_decl.type;
                snprintf(out->struct_type, strlen(out->struct_type), "%s",
                         n->data.struct_decl.name);
                break;
            case N_FORMAL:
                out->datatype = n->data.formal.type;
                snprintf(out->struct_type, strlen(out->struct_type), "%s",
                         n->data.formal.struct_type);
                break;
            case N_MEMBER_DECL:
                out->datatype = n->data.member_decl.type;
                break;
            case N_INTEGER_LITERAL:
                out->datatype = n->data.integer_literal.type;
                break;
            case N_FLOAT_LITERAL:
                out->datatype = n->data.float_literal.type;
                break;
            case N_STRING_LITERAL:
                out->datatype = n->data.string_literal.type;
                break;
            case N_BOOL_LITERAL:
                out->datatype = n->data.bool_literal.type;
                break;
            case N_NIL:
                out->datatype = D_NIL;
                break;
            case N_CALL_EXPR: {
                // First, check if it's builtin
                data_type builtin_type;
                if (is_builtin(n->data.call_expr.func_name, &builtin_type)) {
                    // If so, output its return type
                    out->datatype = builtin_type;
                }
                // Next, check if it's in the symbol table
                else if (is_duplicate(n->data.call_expr.func_name)) {
                    // If so, output its return type
                    type_t tmp = query_symbol_table(n->data.call_expr.func_name);
                    if (tmp.datatype != D_UNKNOWN) {
                        out->datatype = tmp.datatype;
                        if ((strcmp(tmp.struct_type, "NONE") != 0) &&
                            (strlen(tmp.struct_type) > 0)) {
                            // If there's a valid struct_type, set it as well
                            snprintf(out->struct_type, strlen(out->struct_type), "%s",
                                     tmp.struct_type);
                        }
                    }
                }
                // Otherwise, we don't know where this is from
                else {
                    retval = false;
                    type_error("Trying to call an undeclared function", n);
                }
                break;
            }
            case N_IDENT: {
                // First, try the symbol table
                type_t tmp = query_symbol_table(n->data.identifier.name);
                if (tmp.datatype != D_UNKNOWN) {
                    out->datatype = tmp.datatype;
                    if ((strcmp(tmp.struct_type, "NONE") != 0) && (strlen(tmp.struct_type) > 0)) {
                        // If there's a valid struct_type, set it as well
                        snprintf(out->struct_type, strlen(out->struct_type), "%s", tmp.struct_type);
                    }
                } else {
                    // If we didn't find this identifier in the symbol table, assume it hasn't been
                    // seen yet
                    retval = false;
                    type_error("Undeclared identifier", n);
                }
                break;
            }
            default:
                retval = false;
                printf("Requested node does not contain a coercible type\n");
                print_node(n, 0);
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
            type_t a_type;
            type_t b_type;
            bool result_a = get_type(a, &a_type);
            bool result_b = get_type(b, &b_type);

            if (result_a && result_b) {
                // Do the datatypes match?
                if (a_type.datatype == b_type.datatype) {
                    // Are the struct types both NONE?
                    if ((strcmp(a_type.struct_type, "NONE") == 0) &&
                        (strcmp(b_type.struct_type, "NONE") == 0)) {
                        // If yes, we can say both types are compatible.
                        retval = true;
                    } else {
                        // This can probably be incorporated into the above if block
                        if (strcmp(a_type.struct_type, b_type.struct_type) == 0) {
                            retval = true;
                        }
                    }
                }
            }
        } else {
            printf("Unable to access Node B for type coercion\n");
        }
    } else {
        printf("Unable to access Node A for type coercion\n");
    }

    return retval;
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
                type_error("Mismatch between variable declaration and initialized value", n);
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
                b->data_type   = ast_data_type_to_binding_data_type(n->data.function_decl.type);
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
        } else {
            type_error("Function already declared within this scope", n);
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

static void typecheck_binop_expr(node *n) {
    printf("Typechecking bin_op_expr\n");
    if (n != NULL) {
        // First, check if the LHS is in the symbol table
        typecheck(n->data.bin_op_expr.lhs);

        // Then, the RHS
        typecheck(n->data.bin_op_expr.rhs);

        // TODO: Check the operator to make sure the LHS and RHS are compatible with it

        if (!coerce_to(n->data.bin_op_expr.lhs, n->data.bin_op_expr.rhs)) {
            type_error("Type mismatch in binary operator between left-hand expression and "
                       "right-hand expression",
                       n);
        }
    }
}

static void typecheck_assign_expr(node *n) {
    printf("Typechecking assign expr\n");
    if (n != NULL) {
        // First, check if the LHS is in the symbol table
        typecheck(n->data.assign_expr.lhs);

        // Then, the RHS
        typecheck(n->data.assign_expr.rhs);

        if (!coerce_to(n->data.assign_expr.lhs, n->data.assign_expr.rhs)) {
            type_error("Type mismatch in assignment between left-hand expression and right-hand "
                       "expression",
                       n);
        }
    }
}

static void typecheck_call_expr(node *n) {
    printf("Typechecking call_expr\n");
    if (n != NULL) {
        // Check if the function name is a builtin
        data_type builtin_type;
        if (is_builtin(n->data.call_expr.func_name, &builtin_type)) {
            // If yes, process the arguments
        }
        // Check if the function name is in the symbol table
        else if (is_duplicate(n->data.call_expr.func_name)) {
            // If yes, process the arguments
        } else {
            // If no, error
            char msg[MAX_ERROR_LEN] = {0};
            snprintf(msg, sizeof(msg), "Undeclared function: %s", n->data.call_expr.func_name);
            type_error(msg, n);
        }
    }
}

static void typecheck_ident(node *n) {
    printf("Typechecking ident\n");
    if (n != NULL) {
        // Identifiers should be variable names. If it exists in the symbol table, we know it has
        // previously been declared. If not, raise an error
        if (!is_duplicate(n->data.identifier.name)) {
            char msg[MAX_ERROR_LEN] = {0};
            snprintf(msg, sizeof(msg), "Undeclared identifier: %s", n->data.identifier.name);
            type_error(msg, n);
        }
    }
}

static void typecheck_if_stmt(node *n) {
    printf("Typechecking if_stmt\n");
    if (n != NULL) {
        // First, typecheck the test
        typecheck(n->data.if_stmt.test);

        // Then, check the body
        typecheck(n->data.if_stmt.body);

        // Lastly, the else if it exists
        if (n->data.if_stmt.else_stmt != NULL) {
            typecheck(n->data.if_stmt.else_stmt);
        }
    }
}

static void typecheck_literal(node *n) {
    printf("Typechecking literal\n");
    if (n != NULL) {
        switch (n->type) {
            case N_INTEGER_LITERAL:
                if (n->data.integer_literal.type != D_INTEGER) {
                    type_error("Type mismatch between integer literal declaration and value", n);
                }
                break;
            case N_FLOAT_LITERAL:
                if (n->data.float_literal.type != D_FLOAT) {
                    type_error("Type mismatch between float literal declaration and value", n);
                }
                break;
            case N_STRING_LITERAL:
                if (n->data.string_literal.type != D_STRING) {
                    type_error("Type mismatch between string literal declaration and value", n);
                }
                break;
            case N_BOOL_LITERAL:
                if (n->data.bool_literal.type != D_BOOLEAN) {
                    type_error("Type mismatch between boolean literal declaration and value", n);
                }
                break;
            default:
                type_error("Not a valid literal type", n);
                break;
        }
    }
}

static void typecheck_return_stmt(node *n) {
    printf("Typechecking return_stmt\n");
    if (n != NULL) {
        // Check the expression.
        typecheck(n->data.return_stmt.expr);

        // TODO: We'll also need to be clever and make sure the eventual type
        // of the expression matches the return type of the parent function of this return.
        // This will involve looking into the symbol table and finding the top-level for our scope.
        // The top-level scope for a return statement is a function.

        // Find the binding for our function
        hashtable *ht = symbol_table->table;
        for (int idx = 0; idx < MAX_SLOTS; idx++) {
            if (vector_length(ht->slots[idx]) <= 0) {
                continue;
            } else {
                vector *vec = ht->slots[idx];

                vecnode *vn = vec->head;

                while (vn != NULL) {
                    binding_t *b = (binding_t *)vn->data;
                    if (b->object_type == SYM_OTYPE_FUNCTION) {
                        // Now check if the return stmt type matches the function return type
                        type_t return_type;
                        if (get_type(n->data.return_stmt.expr, &return_type)) {
                            if (return_type.datatype != sym_data_to_data_type(b->data_type)) {
                                printf("b->name: %s, b->data_type: %d\n", b->name, b->data_type);
                                char msg[MAX_ERROR_LEN] = {'\0'};
                                snprintf(msg, sizeof(msg),
                                         "Mismatch between function return type: %d and return "
                                         "statement expression type %d",
                                         return_type.datatype, sym_data_to_data_type(b->data_type));
                                type_error(msg, n);
                            }
                            return;
                        }
                    }

                    vn = vn->next;
                }
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
            case N_FUNC_DECL:
                typecheck_func_decl(ast);
                break;
            case N_CALL_EXPR:
                typecheck_call_expr(ast);
                break;
            case N_FORMAL:
                typecheck_formal(ast);
                break;
            case N_IDENT:
                typecheck_ident(ast);
                break;
            case N_BINOP_EXPR:
                typecheck_binop_expr(ast);
                break;
            case N_ASSIGN_EXPR:
                typecheck_assign_expr(ast);
                break;
            case N_IF_STMT:
                typecheck_if_stmt(ast);
                break;
            case N_INTEGER_LITERAL:
            case N_FLOAT_LITERAL:
            case N_STRING_LITERAL:
            case N_BOOL_LITERAL:
                typecheck_literal(ast);
                break;
            case N_RETURN_STMT:
                typecheck_return_stmt(ast);
                break;
            case N_LABEL_DECL:
            case N_GOTO_STMT:
            case N_STRUCT_DECL:
            case N_STRUCT_ACCESS_EXPR:
            case N_ARRAY_INIT_EXPR:
            case N_ARRAY_ACCESS_EXPR:
            case N_MEMBER_DECL:
            case N_NIL:
            case N_WHILE_STMT:
            case N_EMPTY_EXPR:
            case N_NEG_EXPR:
            case N_NOT_EXPR:
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

// Check if an identifier corresponds to a builtin function.
// If it does, also output its type.
static bool is_builtin(const char *ident, data_type *type) {
    bool retval = false;

    if (ident == NULL) {
        log_error("Unable to access identifier to check if it is a builtin function");
    }

    builtin_t getstdin = {.name = "getstdin", .type = D_STRING};
    builtin_t println  = {.name = "println", .type = D_VOID};

    builtin_t built_ins[N_BUILTINS] = {getstdin, println};

    for (int i = 0; i < N_BUILTINS; i++) {
        if (strcmp(ident, built_ins[i].name) == 0) {
            *type  = built_ins[i].type;
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
