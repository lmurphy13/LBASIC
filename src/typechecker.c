/**
 * LBASIC Semantic Analyzer Module
 * File: typechecker.c
 * Author: Liam M. Murphy
 */

#include "typechecker.h"

#include "ast.h"
#include "error.h"
#include "symtab.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static symtab_t *symbol_table = NULL;

//static void build_symbol_table(symtab_t *symbol_table, node *ast);

static void do_typecheck(node *ast);
static void typecheck_program(node *ast);
static void typecheck_block_stmt(node *ast);
static void typecheck_var_decl(node *ast);
static void typecheck_func_decl(node *ast);
static void typecheck_call_expr(node *ast);
static void typecheck_formal(node *ast);
static void typecheck_ident(node *ast);
static void typecheck_binop_expr(node *ast);
static void typecheck_assign_expr(node *ast);
static void typecheck_if_stmt(node *ast);
static void typecheck_literal(node *ast);
static void typecheck_return_stmt(node *ast);
static void typecheck_nil(node *ast);
static void typecheck_struct_decl(node *ast);
static void typecheck_struct_access(node *ast);
static void typecheck_label_decl(node *ast);
static void typecheck_goto_stmt(node *ast);
static void typecheck_array_init_expr(node *ast);
static void typecheck_array_access_expr(node *ast);
static void typecheck_while_stmt(node *ast);
static void typecheck_empty_expr(node *ast);
static void typecheck_neg_expr(node *ast);
static void typecheck_not_expr(node *ast);
static bool match_types(node *a, node *b, type_t *type_a, type_t *type_b);

// TODO: Improve
static void type_error(const char *str, node *n) {
    printf("Type Error: %s\n", str);
    print_node(n, 0);
    exit(TYPE_ERROR);
}

void print_symbol_tables(void) {}

void typecheck(node *ast) {
    if (ast != NULL) {

        // Pass 1: Build symbol table
        symbol_table = symtab_new();

        if (symbol_table == NULL) {
            log_error("Unable to allocate symbol table within typechecker");
        }

        debug("Allocated symbol table for global scope (scope=%d)", symbol_table->scope);

        //build_symbol_table(symbol_table, ast);

        do_typecheck(ast);
    }
}

/*
static void build_symbol_table(symtab_t *symbol_table, node *ast) {
    if ((NULL != symbol_table) && (NULL != ast)) {
        debug("Building symbol table");

        if (ast->data.program.statements->head != NULL) {
            vecnode *vn = ast->data.program.statements->head;
            while (vn != NULL) {
                node *n = vn->data;

                do_typecheck(n);

                vn = vn->next;
            }
        }
    }
}
*/

static void do_typecheck(node *ast) {
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
            typecheck_label_decl(ast);
            break;
        case N_GOTO_STMT:
            typecheck_goto_stmt(ast);
            break;
        case N_ARRAY_INIT_EXPR:
            typecheck_array_init_expr(ast);
            break;
        case N_ARRAY_ACCESS_EXPR:
            typecheck_array_access_expr(ast);
            break;
        case N_WHILE_STMT:
            typecheck_while_stmt(ast);
            break;
        case N_EMPTY_EXPR:
            typecheck_empty_expr(ast);
            break;
        case N_NEG_EXPR:
            typecheck_neg_expr(ast);
            break;
        case N_NOT_EXPR:
            typecheck_not_expr(ast);
            break;
        default:
            type_error("Unknown node type", ast);
            break;
    }
}

// Use for leaf nodes
static type_t get_type(node *n) {
    if (NULL == n) {
        log_error("Unable to access node for type checking");
    }

    type_t type = { 0 };

    switch (n->type) {
        case N_IDENT:
            // Get identifier type from the symbol table
            binding_t *ident_binding = symtab_lookup(symbol_table, n->data.identifier.name);
            if (NULL != ident_binding) {
                switch (ident_binding->symbol_type) {
                    case SYMBOL_TYPE_FUNCTION:
                        //type.datatype = ident_binding->data.function_type.return_type;
                        //type.is_function = true;
                        log_error("SYMBOL_TYPE_FUNCTION not implemented yet: %s", ident_binding->name);
                        break;
                    case SYMBOL_TYPE_VARIABLE:
                        type.datatype = ident_binding->data.variable_type.type;
                        type.is_array = ident_binding->data.variable_type.is_array_type;
                        break;
                    case SYMBOL_TYPE_STRUCTURE:
                        log_error("SYMBOL_TYPE_STRUCTURE not implemented yet: %s", ident_binding->name);
                        break;
                    case SYMBOL_TYPE_MEMBER:
                        log_error("SYMBOL_TYPE_MEMBER not implemented yet: %s", ident_binding->name);
                        break;
                    case SYMBOL_TYPE_UNKNOWN:
                    default:
                        log_error("Unknown symbol type %d", ident_binding->symbol_type);
                }
            }
            break;
        case N_INTEGER_LITERAL:
            type.datatype = n->data.integer_literal.type;
            break;
        case N_FLOAT_LITERAL:
            type.datatype = n->data.float_literal.type;
            break;
        case N_STRING_LITERAL:
            type.datatype = n->data.string_literal.type;
            break;
        case N_BOOL_LITERAL:
            type.datatype = n->data.bool_literal.type;
            break;
        default:
            log_error("Type %d not implemented yet", n->type);
    }

    return type;
}

static bool match_types(node *a, node *b, type_t *type_a, type_t *type_b) {
    bool result = false;

    if (NULL == a) {
        log_error("Unable to access node A for type matching");
    }

    if (NULL == b) {
        log_error("Unable to access node B for type matching");
    }

    debug("Node A is of node type %d", a->type);
    debug("Node B is of node type %d", b->type);

    type_t a_type = get_type(a);
    type_t b_type = get_type(b);

    // TODO become more clever with structs, arrays, boolean expressions, etc.
    result = (a_type.datatype == b_type.datatype);

    *type_a = a_type;
    *type_b = b_type;

    return result;
}

static void typecheck_program(node *ast) {
    // assert(false && "Not yet implemented");
    printf("Typechecking program\n");
    if (ast != NULL) {
        // Typecheck children
        if (ast->data.program.statements->head != NULL) {
            vecnode *vn = ast->data.program.statements->head;
            while (vn != NULL) {
                node *n = vn->data;

                do_typecheck(n);

                vn = vn->next;
            }
        }
    }
}

static void typecheck_block_stmt(node *ast) { assert(false && "Not yet implemented"); }

static void typecheck_var_decl(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for typechecking", __FUNCTION__);
    }

    // Check if binding already exists within current scope
    binding_t *existing_binding = symtab_lookup(symbol_table, ast->data.var_decl.name);
    if (NULL != existing_binding) {
        char err_msg[MAX_ERROR_LEN] = { 0 };
        snprintf(err_msg, MAX_ERROR_LEN, "Redefinition of '%s'", existing_binding->name);
        type_error(err_msg, ast);
    }

    // Create new binding
    binding_t *new_binding = mk_binding(SYMBOL_TYPE_VARIABLE);
    if (NULL == new_binding) {
        log_error("%s(): Unable to create new binding_t", __FUNCTION__);
    }

    // Populate binding data
    snprintf(new_binding->name, MAX_LITERAL, ast->data.var_decl.name);
    snprintf(new_binding->data.variable_type.struct_type, MAX_LITERAL, ast->data.var_decl.struct_type);

    new_binding->data.variable_type.type = ast->data.var_decl.type;
    new_binding->data.variable_type.is_array_type = ast->data.var_decl.is_array;
    new_binding->data.variable_type.is_struct_type = ast->data.var_decl.is_struct;
    new_binding->data.variable_type.num_dimensions = ast->data.var_decl.num_dimensions;

    // Insert binding into symbol table
    symtab_insert(symbol_table, new_binding);
    print_symbol_table(symbol_table);
}

static void typecheck_func_decl(node *ast) { assert(false && "Not yet implemented"); }

static void typecheck_call_expr(node *ast) { assert(false && "Not yet implemented"); }

static void typecheck_formal(node *ast) { assert(false && "Not yet implemented"); }

static void typecheck_ident(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for typechecking", __FUNCTION__);
    }

    // Check if the identifier is within the symbol table
    // For now, check only global scope
    binding_t *ident_binding = symtab_lookup(symbol_table, ast->data.identifier.name);
    if (NULL == ident_binding) {
        log_error("Undeclared identifier '%s'", ast->data.identifier.name);
    }
}

static void typecheck_binop_expr(node *ast) { assert(false && "Not yet implemented"); }

static void typecheck_assign_expr(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for typechecking", __FUNCTION__);
    }

    // Visit LHS
    do_typecheck(ast->data.assign_expr.lhs);

    // Visit RHS
    do_typecheck(ast->data.assign_expr.rhs);

    type_t lhs_type = { 0 };
    type_t rhs_type = { 0 };

    // Check if LHS type and RHS type match
    if (!match_types(ast->data.assign_expr.lhs, ast->data.assign_expr.rhs, &lhs_type, &rhs_type)) {
        char err_msg[MAX_ERROR_LEN] = { 0 };
        snprintf(err_msg, MAX_ERROR_LEN, "Type mismatch. Expected '%s'. Got '%s'.",
            type_to_str(lhs_type.datatype), type_to_str(rhs_type.datatype));
        type_error(err_msg, ast);
    }

}

static void typecheck_if_stmt(node *ast) { assert(false && "Not yet implemented"); }

static void typecheck_literal(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for typechecking", __FUNCTION__);
    }

    // Nothing to do
}

static void typecheck_return_stmt(node *ast) { assert(false && "Not yet implemented"); }

static void typecheck_nil(node *ast) { assert(false && "Not yet implemented"); }

static void typecheck_struct_decl(node *ast) { assert(false && "Not yet implemented"); }

static void typecheck_struct_access(node *ast) { assert(false && "Not yet implemented"); }

static void typecheck_label_decl(node *ast) { assert(false && "Not yet implemented"); }

static void typecheck_goto_stmt(node *ast) { assert(false && "Not yet implemented"); }

static void typecheck_array_init_expr(node *ast) { assert(false && "Not yet implemented"); }

static void typecheck_array_access_expr(node *ast) { assert(false && "Not yet implemented"); }

static void typecheck_while_stmt(node *ast) { assert(false && "Not yet implemented"); }

static void typecheck_empty_expr(node *ast) { assert(false && "Not yet implemented"); }

static void typecheck_neg_expr(node *ast) { assert(false && "Not yet implemented"); }

static void typecheck_not_expr(node *ast) { assert(false && "Not yet implemented"); }
