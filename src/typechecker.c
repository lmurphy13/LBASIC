/**
 * LBASIC Semantic Analyzer Module
 * File: typechecker.c
 * Author: Liam M. Murphy
 */

#include "typechecker.h"

#include "ast.h"
#include "error.h"
#include "symtab.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define N_BUILTINS 4

typedef struct builtin_s {
    char name[MAX_LITERAL];
    data_type type;
} builtin_t;

// Globals

// Main stack of symbol tables
static vector *stack = NULL;

// Auxiliary stack of symbol tables used when needing to look through multiple scopes
// and return to the "context" we started with
static vector *aux = NULL;

static symtab_t *curr     = NULL;
static bool created_table = false;
static unsigned int level = 1;

// Prototypes
static bool is_duplicate(symtab_t *scope, const char *ident);
static bool is_builtin(const char *ident, data_type *type);
static void begin_new_scope(void);
static void begin_scope(void);
static void end_scope(void);
static void pop_head(void);
static void restore_head(void);
static void cleanup_aux(void);

static void type_error(const char *str, node *n) {
    printf("Type Error! Node type: %d\n", n->type);
    printf("%s\n", str);
    print_node(n, INDENT_WIDTH);

#if defined(DEBUG)
    printf("\n");
    // Symbol Table Dump
    print_symbol_tables();
#endif
    exit(1);
}

static type_t query_symbol_table(symtab_t *scope, const char *ident) {
    type_t retval;
    memset(&retval, 0, sizeof(type_t));
    retval.datatype = D_UNKNOWN;
    snprintf(retval.struct_type, strlen(retval.struct_type), "NONE");

    if (ident != NULL) {
        if (scope != NULL) {
            // Search hashtable for symbols matching ident
            hashtable *ht = scope->table;
            binding_t *b  = ht_lookup(ht, (char *)ident, ht_compare_binding);

            if (b != NULL) {
                switch (b->symbol_type) {
                    case SYMBOL_TYPE_FUNCTION:
                        retval.datatype = b->data.function_type.return_type;
                        if (b->data.function_type.is_struct_type) {
                            snprintf(retval.struct_type, strlen(retval.struct_type), "%s",
                                     b->data.function_type.struct_type);
                        }
                        break;
                    case SYMBOL_TYPE_VARIABLE:
                        retval.datatype = b->data.variable_type.type;
                        if (b->data.variable_type.is_struct_type) {
                            snprintf(retval.struct_type, strlen(retval.struct_type), "%s",
                                     b->data.variable_type.struct_type);
                        }
                        break;
                    case SYMBOL_TYPE_STRUCTURE:
                        snprintf(retval.struct_type, strlen(retval.struct_type), "%s",
                                 b->data.structure_type.struct_type);
                        break;
                    case SYMBOL_TYPE_MEMBER:
                        retval.datatype = b->data.member_type.type;
                        if (b->data.member_type.is_struct_type) {
                            snprintf(retval.struct_type, strlen(retval.struct_type), "%s",
                                     b->data.member_type.struct_type);
                        }
                        break;
                    case SYMBOL_TYPE_UNKNOWN:
                    default:
                        type_error("Unknown symbol type", NULL);
                        break;
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
            case N_STRUCT_ACCESS_EXPR: {
                // Search through the stack
                // First, check if the variable name exists in-scope
                if (is_duplicate(curr, n->data.struct_access.name)) {
                    // This is fine. This means the variable exists

                    // Look in previous scopes until we hit the nearest declaration
                    // matching the name, and verify it is a struct
                    symtab_t *curr_scope = curr;
                    symtab_t *rem_scope  = curr;

                    printf("Looking in level %d\n", curr_scope->level);

                    do {
                        if (is_duplicate(curr_scope, n->data.struct_access.member_name)) {
                            printf("Found struct member name in scope level %d\n",
                                   curr_scope->level);
                            type_t member_type =
                                query_symbol_table(curr, n->data.struct_access.member_name);
                            if (member_type.datatype != D_UNKNOWN) {
                                out->datatype = member_type.datatype;
                                if ((strcmp(member_type.struct_type, "NONE") != 0) &&
                                    (strlen(member_type.struct_type) > 0)) {
                                    // If there's a valid struct_type, set it as well
                                    snprintf(out->struct_type, strlen(out->struct_type), "%s",
                                             member_type.struct_type);
                                }
                            }
                            break;
                        } else {
                            pop_head();

                            vecnode *v = stack->head;
                            if (v != NULL) {
                                symtab_t *s = v->data;

                                if (s != NULL) {
                                    curr_scope = s;
                                }
                            } else {
                                printf("Nothing here! LIAM \n");
                                break;
                            }
                        }

                    } while (curr_scope != NULL);

                    if (curr_scope == NULL) {
                        // We didn't find the variable
                        char msg[MAX_ERROR_LEN] = {0};
                        snprintf(msg, sizeof(msg), "Undeclared struct identifier: %s",
                                 n->data.struct_access.name);
                        type_error(msg, n);
                    }

                    // Reset the symbol table to what we had before
                    vecnode *stack_head_vn = vector_top(stack);
                    symtab_t *stack_head   = (symtab_t *)stack_head_vn->data;

                    while (stack_head != rem_scope) {
                        printf("sh: %p | ah: %p\n", stack_head, rem_scope);
                        printf("stack level: %d\n", stack_head->level);

                        if (stack_head->level == 0) {
                            break;
                        }
                        restore_head();
                    }

                } else {
                    // Look in previous scopes until we hit the nearest declaration
                    // matching the name, and verify it is a struct
                    symtab_t *curr_scope = curr;
                    symtab_t *rem_scope  = curr;
                    printf("Looking in level %d\n", curr_scope->level);

                    do {
                        if (is_duplicate(curr_scope, n->data.struct_access.name)) {
                            printf("Found struct variable name in scope level %d\n",
                                   curr_scope->level);

                            // Next, check if the struct has this member (its type is in the symbol
                            // table, and will be checked elsewhere)
                            break;
                        } else {
                            pop_head();

                            vecnode *v = stack->head;
                            if (v != NULL) {
                                symtab_t *s = v->data;

                                if (s != NULL) {
                                    curr_scope = s;
                                }
                            } else {
                                printf("Nothing here! LIAM \n");
                                break;
                            }
                        }

                    } while (curr_scope != NULL);

                    if (curr_scope == NULL) {
                        // We didn't find the variable
                        char msg[MAX_ERROR_LEN] = {0};
                        snprintf(msg, sizeof(msg), "Undeclared struct identifier: %s",
                                 n->data.struct_access.name);
                        type_error(msg, n);
                    }

                    // Reset the symbol table to what we had before
                    vecnode *stack_head_vn = vector_top(stack);
                    symtab_t *stack_head   = (symtab_t *)stack_head_vn->data;

                    while (stack_head != rem_scope) {
                        restore_head();
                    }
                }
                break;
            }
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
                // Next, check if it's declared in the current scope
                else if (is_duplicate(curr, n->data.call_expr.func_name)) {
                    // If so, output its return type
                    type_t tmp = query_symbol_table(curr, n->data.call_expr.func_name);
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
                // Otherwise, look within parent scopes
                else {
                    // Check the previous scopes
                    pop_head();
                    symtab_t *curr_scope = curr;
                    printf("Looking in level %d\n", curr_scope->level);

                    do {
                        if (is_duplicate(curr_scope, n->data.call_expr.func_name)) {
                            printf("Found function name in scope level %d\n", curr_scope->level);
                            // If so, output its return type
                            type_t tmp =
                                query_symbol_table(curr_scope, n->data.call_expr.func_name);
                            if (tmp.datatype != D_UNKNOWN) {
                                out->datatype = tmp.datatype;
                                if ((strcmp(tmp.struct_type, "NONE") != 0) &&
                                    (strlen(tmp.struct_type) > 0)) {
                                    // If there's a valid struct_type, set it as well
                                    snprintf(out->struct_type, strlen(out->struct_type), "%s",
                                             tmp.struct_type);
                                }
                            }
                            retval = true;
                            break;
                        } else {
                            pop_head();
                            curr_scope = curr;
                        }

                    } while (curr_scope != NULL);

                    if (curr_scope == NULL) {
                        // We didn't find the function
                        char msg[MAX_ERROR_LEN] = {0};
                        snprintf(msg, sizeof(msg), "Trying to call an undeclared function: %s",
                                 n->data.call_expr.func_name);
                        retval = false;
                        type_error(msg, n);
                    }
                }
                break;
            }
            case N_IDENT: {
                // First, try the symbol table
                type_t tmp = query_symbol_table(curr, n->data.identifier.name);
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
            case N_BINOP_EXPR: {
                // The "type' of binary operator expressions depends on the operator.
                // Arithmetic operators will return a numerical type, logical operators will return
                // a boolean
                switch (n->data.bin_op_expr.operator) {
                    case T_PLUS:
                    case T_MINUS:
                    case T_MUL:
                    case T_DIV:
                    case T_MOD: {
                        // return INTEGER or FLOAT
                        type_t lhs_type;
                        type_t rhs_type;
                        if (get_type(n->data.bin_op_expr.lhs, &lhs_type)) {
                            if (get_type(n->data.bin_op_expr.rhs, &rhs_type)) {
                                printf("LHS TYPE: %s (%d)\n", type_to_str(lhs_type.datatype),
                                       lhs_type.datatype);
                                printf("RHS TYPE: %s (%d)\n", type_to_str(rhs_type.datatype),
                                       rhs_type.datatype);
                            }
                        }
                        break;
                    }
                    case T_LT:
                    case T_LE:
                    case T_GT:
                    case T_GE:
                    case T_EQ:
                    case T_NE:
                    case T_AND:
                    case T_OR:
                        // return BOOLEAN
                        out->datatype = D_BOOLEAN;
                        break;
                    default: {
                        char msg[MAX_ERROR_LEN] = {'\0'};
                        snprintf(msg, sizeof(msg), "Unknown operator: %d",
                                 n->data.bin_op_expr.operator);
                        retval = false;
                        type_error(msg, n);
                    }
                }
            } break;
            default:
                retval = false;
                printf("Requested node does not contain a coercible type\n");
                print_node(n, 0);
                break;
        }

    } else {
        log_error("Unable to access node for type retrieval");
    }

    printf("Type was %s\n", type_to_str(out->datatype));

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
        printf("I am in scope %d\n", curr->level);
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

        // Do we have a duplicate in the current scope?
        // If this is scope 0, then no problem. If this is a nested scope,
        // then check only within this scope. (shadowing)
        if (!is_duplicate(curr, n->data.var_decl.name)) {

            // There may not be an immediately assigned value
            if (n->data.var_decl.value != NULL) {
                if (!coerce_to(n, n->data.var_decl.value)) {
                    type_error("Mismatch between variable declaration and initialized value", n);
                }
            }

            binding_t *b = mk_binding(SYMBOL_TYPE_VARIABLE);
            if (b != NULL) {
                snprintf(b->name, sizeof(b->name), "%s", n->data.var_decl.name);

                if (n->data.var_decl.is_struct) {
                    b->data.variable_type.is_struct_type = true;
                    snprintf(b->data.variable_type.struct_type,
                             sizeof(b->data.variable_type.struct_type), "%s",
                             n->data.var_decl.struct_type);
                }

                b->data.variable_type.type           = n->data.var_decl.type;
                b->data.variable_type.is_array_type  = n->data.var_decl.is_array;
                b->data.variable_type.num_dimensions = n->data.var_decl.num_dimensions;

                ht_insert(curr->table, b->name, b);
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
        if (!is_duplicate(curr, n->data.function_decl.name)) {
            binding_t *b = mk_binding(SYMBOL_TYPE_FUNCTION);

            if (b != NULL) {
                snprintf(b->name, sizeof(b->name), "%s", n->data.function_decl.name);

                b->data.function_type.return_type    = n->data.function_decl.type;
                b->data.function_type.is_array_type  = n->data.function_decl.is_array;
                b->data.function_type.num_dimensions = n->data.function_decl.num_dimensions;
                b->data.function_type.is_struct_type = n->data.function_decl.is_struct;

                if (n->data.function_decl.is_struct) {
                    snprintf(b->data.function_type.struct_type,
                             sizeof(b->data.function_type.struct_type), "%s",
                             n->data.function_decl.struct_type);
                }

                b->data.function_type.num_args = vector_length(n->data.function_decl.formals);

                ht_insert(curr->table, b->name, b);
            }

            // Enter a new scope here
            begin_scope();

            if (vector_length(n->data.function_decl.formals) > 0) {
                vector *vec = n->data.function_decl.formals;

                vecnode *vn = vec->head;

                while (vn != NULL) {
                    typecheck(vn->data);

                    vn = vn->next;
                }
            }

            typecheck(n->data.function_decl.body);

            // Leave the scope here
            end_scope();

        } else {
            type_error("Function already declared within this scope", n);
        }
    }
}

static void typecheck_formal(node *n) {
    printf("Typechecking formal\n");
    if (n != NULL) {
        if (!is_duplicate(curr, n->data.formal.name)) {
            binding_t *b = mk_binding(SYMBOL_TYPE_VARIABLE);

            if (b != NULL) {
                snprintf(b->name, sizeof(b->name), "%s", n->data.formal.name);

                if (n->data.formal.is_struct) {
                    b->data.variable_type.is_struct_type = true;
                    snprintf(b->data.variable_type.struct_type,
                             sizeof(b->data.variable_type.struct_type), "%s",
                             n->data.formal.struct_type);
                }

                b->data.variable_type.type           = n->data.formal.type;
                b->data.variable_type.is_array_type  = n->data.formal.is_array;
                b->data.variable_type.num_dimensions = n->data.formal.num_dimensions;

                ht_insert(curr->table, b->name, b);
            }
        }
    }
}

static void typecheck_binop_expr(node *n) {
    printf("Typechecking bin_op_expr\n");
    if (n != NULL) {
        type_t lhs_type;
        type_t rhs_type;

        // First, check if the LHS is in the symbol table
        typecheck(n->data.bin_op_expr.lhs);

        // Then, the RHS
        typecheck(n->data.bin_op_expr.rhs);

        // TODO: Check the operator to make sure the LHS and RHS are compatible with it
        // The "type' of binary operator expressions depends on the operator.
        // Arithmetic operators will return a numerical type, logical operators will return a
        // boolean
        if (get_type(n->data.bin_op_expr.lhs, &lhs_type)) {
            if (get_type(n->data.bin_op_expr.rhs, &rhs_type)) {
                //                printf("LHS TYPE: %d\n", lhs_type.datatype);
                //                printf("RHS TYPE: %d\n", rhs_type.datatype);
                printf("LHS TYPE: %s (%d)\n", type_to_str(lhs_type.datatype), lhs_type.datatype);
                printf("RHS TYPE: %s (%d)\n", type_to_str(rhs_type.datatype), rhs_type.datatype);

                switch (n->data.bin_op_expr.operator) {
                    case T_PLUS:
                    case T_MINUS:
                    case T_MUL:
                    case T_DIV:
                    case T_MOD: {
                        // INTEGER or FLOAT
                        if (!((lhs_type.datatype == D_INTEGER) || (lhs_type.datatype == D_FLOAT))) {
                            char msg[MAX_ERROR_LEN] = {'\0'};
                            snprintf(msg, sizeof(msg),
                                     "Illegal LHS operand type %s (%d) applied with arithmetic "
                                     "operator.",
                                     type_to_str(lhs_type.datatype), lhs_type.datatype);
                            type_error(msg, n);
                        }

                        if (!((rhs_type.datatype == D_INTEGER) || (rhs_type.datatype == D_FLOAT))) {
                            char msg[MAX_ERROR_LEN] = {'\0'};
                            snprintf(msg, sizeof(msg),
                                     "Illegal RHS operand type %s (%d) applied with arithmetic "
                                     "operator.",
                                     type_to_str(rhs_type.datatype), rhs_type.datatype);
                            type_error(msg, n);
                        }
                    } break;
                    case T_LT:
                    case T_LE:
                    case T_GT:
                    case T_GE:
                    case T_EQ:
                    case T_NE:
                    case T_AND:
                    case T_OR: {
                        if (!((lhs_type.datatype == D_INTEGER) || (lhs_type.datatype == D_FLOAT) ||
                              (lhs_type.datatype == D_STRING) ||
                              (lhs_type.datatype == D_BOOLEAN))) {
                            char msg[MAX_ERROR_LEN] = {'\0'};
                            snprintf(
                                msg, sizeof(msg),
                                "Illegal LHS operand type %s (%d) applied with logical operator.",
                                type_to_str(lhs_type.datatype), lhs_type.datatype);
                            type_error(msg, n);
                        }

                        if (!((rhs_type.datatype == D_INTEGER) || (rhs_type.datatype == D_FLOAT) ||
                              (rhs_type.datatype == D_STRING) ||
                              (rhs_type.datatype == D_BOOLEAN))) {
                            char msg[MAX_ERROR_LEN] = {'\0'};
                            snprintf(
                                msg, sizeof(msg),
                                "Illegal RHS operand type %s (%d) applied with logical operator.",
                                type_to_str(rhs_type.datatype), rhs_type.datatype);
                            type_error(msg, n);
                        }
                    } break;
                    default: {
                        char msg[MAX_ERROR_LEN] = {'\0'};
                        snprintf(msg, sizeof(msg), "Unknown operator: %d",
                                 n->data.bin_op_expr.operator);
                        type_error(msg, n);
                    }
                }
                if (!coerce_to(n->data.bin_op_expr.lhs, n->data.bin_op_expr.rhs)) {
                    char msg[MAX_ERROR_LEN] = {'\0'};

                    snprintf(msg, sizeof(msg),
                             "Type mismatch in binary operator between left-hand expression %s"
                             "(%d) and "
                             "right-hand expression %s (%d)",
                             type_to_str(lhs_type.datatype), lhs_type.datatype,
                             type_to_str(rhs_type.datatype), rhs_type.datatype);
                    type_error(msg, n);
                }
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
            if (n->data.call_expr.args != NULL) {
                vecnode *vn = n->data.call_expr.args->head;

                while (vn != NULL) {
                    typecheck(vn->data);

                    vn = vn->next;
                }
            }
        }
        // Check if the function name is in the current scope
        else if (is_duplicate(curr, n->data.call_expr.func_name)) {
            // If yes, process the arguments if they exist
            if (n->data.call_expr.args != NULL) {
                vecnode *vn = n->data.call_expr.args->head;

                while (vn != NULL) {
                    typecheck(vn->data);

                    vn = vn->next;
                }
            }

        } else {
            // Check the previous scopes
            symtab_t *curr_scope = (symtab_t *)(((vecnode *)stack->head)->data);
            printf("Looking in level %d\n", curr_scope->level);

            do {
                if (is_duplicate(curr_scope, n->data.call_expr.func_name)) {
                    printf("Found function name in scope level %d\n", curr_scope->level);
                    // Process the arguments if they exist
                    if (n->data.call_expr.args != NULL) {
                        vecnode *vn = n->data.call_expr.args->head;

                        while (vn != NULL) {
                            typecheck(vn->data);

                            vn = vn->next;
                        }
                        break;
                    }
                } else {
                    // curr_scope = symtab_prev(curr_scope);
                    pop_head();
                    curr_scope = (symtab_t *)(((vecnode *)stack->head)->data);
                }

            } while (curr_scope != NULL);

            if (curr_scope == NULL) {
                // We didn't find the function
                char msg[MAX_ERROR_LEN] = {0};
                snprintf(msg, sizeof(msg), "Undeclared function: %s", n->data.call_expr.func_name);
                type_error(msg, n);
            }

            // Cleanup
            cleanup_aux();
        }
    }
}

static void typecheck_ident(node *n) {
    printf("Typechecking ident\n");
    if (n != NULL) {
        // Identifiers should be variable names. If it exists in the symbol table, we know it has
        // previously been declared. If not, raise an error
        if (is_duplicate(curr, n->data.identifier.name)) {
            //            char msg[MAX_ERROR_LEN] = {0};
            //            snprintf(msg, sizeof(msg), "Undeclared identifier: %s",
            //            n->data.identifier.name); type_error(msg, n);
        } else {
            // Look in previous scopes until we hit the nearest declaration
            symtab_t *curr_scope = curr;
            printf("Looking in level %d\n", curr_scope->level);

            do {
                if (is_duplicate(curr_scope, n->data.identifier.name)) {
                    printf("Found function name in scope level %d\n", curr_scope->level);
                    break;
                } else {
                    //                    curr_scope = symtab_prev(curr_scope);
                    pop_head();

                    vecnode *v = stack->head;
                    if (v != NULL) {
                        symtab_t *s = v->data;

                        if (s != NULL) {
                            curr_scope = s;
                        }
                    } else {
                        printf("Nothing here!\n");
                        break;
                    }
                }

            } while (curr_scope != NULL);

            if (curr_scope == NULL) {
                // We didn't find the function
                char msg[MAX_ERROR_LEN] = {0};
                snprintf(msg, sizeof(msg), "Undeclared identifier: %s", n->data.identifier.name);
                type_error(msg, n);
            }
        }
    }
}

static void typecheck_if_stmt(node *n) {
    printf("Typechecking if_stmt\n");
    if (n != NULL) {
        // First, typecheck the test
        typecheck(n->data.if_stmt.test);

        begin_scope();

        // Then, check the body
        typecheck(n->data.if_stmt.body);

        end_scope();

        // Lastly, the else if it exists
        if (n->data.if_stmt.else_stmt != NULL) {
            begin_new_scope();

            typecheck(n->data.if_stmt.else_stmt);

            end_scope();
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
        if (n->data.return_stmt.expr == NULL) {
            // Empty return, so no need to typecheck.
        } else {
            typecheck(n->data.return_stmt.expr);

            // TODO: We'll also need to be clever and make sure the eventual type
            // of the expression matches the return type of the parent function of this return.
            // This will involve looking into the symbol table and finding the top-level for our
            // scope. The top-level scope for a return statement is a function.

            // Find the binding for our function
            symtab_t *st  = (symtab_t *)(((vecnode *)stack->head)->data);
            hashtable *ht = st->table;
            for (int idx = 0; idx < MAX_SLOTS; idx++) {
                if (vector_length(ht->slots[idx]) <= 0) {
                    continue;
                } else {
                    vector *vec = ht->slots[idx];

                    vecnode *vn = vec->head;

                    while (vn != NULL) {
                        binding_t *b = (binding_t *)vn->data;
                        if (b->symbol_type == SYMBOL_TYPE_FUNCTION) {
                            // Now check if the return stmt type matches the function return type
                            type_t return_type;
                            if (get_type(n->data.return_stmt.expr, &return_type)) {
                                if (return_type.datatype != b->data.function_type.return_type) {
                                    printf("b->name: %s, b->return_type: %d\n", b->name,
                                           b->data.function_type.return_type);
                                    char msg[MAX_ERROR_LEN] = {'\0'};
                                    snprintf(msg, sizeof(msg),
                                             "Mismatch between function return type: %d and return "
                                             "statement expression type %d",
                                             b->data.function_type.return_type,
                                             return_type.datatype);
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
}

static void typecheck_nil(node *n) {
    printf("Typechecking nil\n");
    if (n != NULL) {
        // Nil (NULL)
    }
}

// TODO: Go back into the parser and AST and capture members that could be arrays or structs
static void typecheck_struct_decl(node *n) {
    printf("Typechecking struct_decl\n");
    if (n != NULL) {
        binding_t *struct_binding = mk_binding(SYMBOL_TYPE_STRUCTURE);
        if (struct_binding != NULL) {
            snprintf(struct_binding->name, sizeof(struct_binding->name), "%s",
                     n->data.struct_decl.name);
            struct_binding->data.structure_type.num_members =
                vector_length(n->data.struct_decl.members);

            vecnode *curr_node     = stack->head;
            symtab_t *symbol_table = curr_node->data;
            ht_insert(symbol_table->table, struct_binding->name, struct_binding);

            // Add members to the symbol table, if they exist
            if (struct_binding->data.structure_type.num_members > 0) {
                if (n->data.struct_decl.members != NULL) {
                    vecnode *vn = n->data.struct_decl.members->head;

                    begin_scope();
                    while (vn != NULL) {
                        node *m = (node *)vn->data;

                        if (m != NULL) {
                            binding_t *member_binding = mk_binding(SYMBOL_TYPE_MEMBER);
                            if (member_binding != NULL) {
                                snprintf(member_binding->name, sizeof(member_binding->name), "%s",
                                         m->data.member_decl.name);
                                member_binding->data.member_type.type = m->data.member_decl.type;
                                // Parent struct name
                                snprintf(member_binding->data.member_type.struct_type,
                                         sizeof(member_binding->data.member_type.struct_type), "%s",
                                         n->data.struct_decl.name);

                                if (!is_duplicate(curr, member_binding->name)) {
                                    ht_insert(curr->table, member_binding->name, member_binding);
                                } else {
                                    char msg[MAX_ERROR_LEN] = {0};
                                    snprintf(msg, sizeof(msg),
                                             "Member already defined in this scope: %s",
                                             m->data.member_decl.name);
                                    type_error(msg, n);
                                }
                            }
                        }

                        vn = vn->next;
                    }
                    end_scope();
                }
            }
        }
    }
}

static void typecheck_struct_access(node *n) {
    printf("Typechecking struct access\n");
    if (n != NULL) {
        // First, check if the variable name exists in-scope
        if (is_duplicate(curr, n->data.struct_access.name)) {
            // This is fine. This means the variable exists
        } else {
            // Look in previous scopes until we hit the nearest declaration
            // matching the name, and verify it is a struct
            symtab_t *curr_scope = curr;
            symtab_t *rem_scope  = curr;
            printf("Looking in level %d\n", curr_scope->level);

            do {
                if (is_duplicate(curr_scope, n->data.struct_access.name)) {
                    printf("Found struct variable name in scope level %d\n", curr_scope->level);

                    // Next, check if the struct has this member (its type is in the symbol table,
                    // and will be checked elsewhere)
                    break;
                } else {
                    pop_head();

                    vecnode *v = stack->head;
                    if (v != NULL) {
                        symtab_t *s = v->data;

                        if (s != NULL) {
                            curr_scope = s;
                        }
                    } else {
                        printf("Nothing here! LIAM \n");
                        break;
                    }
                }

            } while (curr_scope != NULL);

            if (curr_scope == NULL) {
                // We didn't find the variable
                char msg[MAX_ERROR_LEN] = {0};
                snprintf(msg, sizeof(msg), "Undeclared struct identifier: %s",
                         n->data.struct_access.name);
                type_error(msg, n);
            }

            // Reset the symbol table to what we had before
            vecnode *stack_head_vn = vector_top(stack);
            symtab_t *stack_head   = (symtab_t *)stack_head_vn->data;

            while (stack_head != rem_scope) {
                restore_head();
            }
        }
    }
}

void typecheck(node *ast) {
    if (ast != NULL) {
        if (!created_table) {
            // The first symbol table is the global scope
            stack = mk_vector();
            aux   = mk_vector();

            if (stack == NULL) {
                log_error("Unable to create symbol table vector");
            } else {
                symtab_t *st = symtab_new();
                if (st != NULL) {
                    st->level = 0;

                    // Push scope 0 onto the stack
                    vector_prepend(stack, st);
                    curr = stack->head->data;
                    printf("Successfully allocated symbol table\n");
                    created_table = true;
                }
            }

            if (aux == NULL) {
                log_error("Unable to create aux stack vector");
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
            case N_NIL:
                typecheck_nil(ast);
                break;
            case N_STRUCT_DECL:
                typecheck_struct_decl(ast);
                break;
            case N_STRUCT_ACCESS_EXPR:
                typecheck_struct_access(ast);
                break;
            case N_LABEL_DECL:
            case N_GOTO_STMT:
            case N_ARRAY_INIT_EXPR:
            case N_ARRAY_ACCESS_EXPR:
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

// Search for an identifier within the given scope's symbol table
static bool is_duplicate(symtab_t *scope, const char *ident) {
    bool retval = false;

    if (ident == NULL) {
        log_error("Unable to access identifier before symbol table lookup");
    }

    // Examine the scope's table
    if (scope != NULL) {
        // Search hashtable for symbols matching ident
        printf("Now checking symbol table level %d\n", scope->level);

        hashtable *ht = scope->table;
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

    builtin_t getstr   = {.name = "getstr", .type = D_STRING};
    builtin_t getint   = {.name = "getint", .type = D_INTEGER};
    builtin_t getfloat = {.name = "getfloat", .type = D_FLOAT};
    builtin_t println  = {.name = "println", .type = D_VOID};

    builtin_t built_ins[N_BUILTINS] = {getstr, getint, getfloat, println};

    for (int i = 0; i < N_BUILTINS; i++) {
        if (strcmp(ident, built_ins[i].name) == 0) {
            *type  = built_ins[i].type;
            retval = true;
        }
    }

    return retval;
}

static void begin_new_scope() {
    symtab_t *new = symtab_new();

    if (new != NULL) {

        new->level = level++;
        vector_prepend(stack, new);

        vecnode *vn = (vecnode *)vector_top(stack);
        curr        = (symtab_t *)vn->data;

        printf("Created scope level %d\n", curr->level);
    } else {
        log_error("Unable to create new symbol table scope");
    }
}

static void begin_scope() { begin_new_scope(); }

static void end_scope() {
    printf("Now leaving scope level %d\n", curr->level);

    if (curr->level > 0) {
        pop_head();
    } else {
        //        log_error("Cannot leave the global scope");
        printf("Cannot leave the global scope\n");
    }

    printf("Now entering scope level %d\n", curr->level);
}

// Pop the head symbol table from the stack and add it to aux
static void pop_head(void) {
    vecnode *head = vector_pop_head(stack);
    if (head != NULL) {
        vector_prepend(aux, head->data);
        if (stack->head != NULL) {
            curr = stack->head->data;
        }
    }
}

// Take the head from aux and push it back onto the stack
static void restore_head(void) {
    if (vector_length(aux) <= 0) {
        printf("Cannot restore from empty aux\n");
        return;
    }

    vecnode *aux_head = vector_pop_head(aux);
    if (aux_head != NULL) {
        printf("Restoring stack head\n");
        symtab_t *aux_symtab = aux_head->data;

        if (aux_symtab != NULL) {
            vector_prepend(stack, aux_symtab);

            if (stack->head != NULL) {
                vecnode *v = stack->head;
                if (v != NULL) {
                    symtab_t *s = v->data;
                    if (s != NULL) {
                        curr = s;
                        printf("curr level is %d\n", curr->level);
                    }
                }
            }
        }
    } else {
        log_error("Aux head is null!\n");
    }
}

// Push all elements in aux to stack
static void cleanup_aux(void) {
    printf("Cleaning up aux\n");
    printf("aux len: %d\n", vector_length(aux));

    while (aux->head != NULL) {
        symtab_t *tab = (symtab_t *)((vecnode *)aux->head)->data;
        if (tab == NULL) {
            log_error("Unable to access table from aux->head");
        }
        restore_head();
    }

    curr = stack->head->data;
}

void print_symbol_tables() {
    cleanup_aux();
    vecnode *stack_head = stack->head;

    while (stack_head != NULL) {
        symtab_t *st = stack_head->data;

        print_table(st);

        pop_head();
        stack_head = stack->head;
    }
}
