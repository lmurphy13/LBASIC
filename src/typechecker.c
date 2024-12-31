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

// Overall symbol table data structure
static symtab_t *symbol_table = NULL;

// Pointer to the current scope
static symtab_t *curr_scope = NULL;

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

// Optional name argument to assist in typechecking return statements against function return type
// Kind of hacky
static void enter_new_scope(char *name) {
    symtab_t *new_scope = symtab_new();
    if (NULL == new_scope) {
        log_error("Unable to create symbol table for new scope");
    }

    if (NULL != name) {
        snprintf(new_scope->name, MAX_LITERAL, name);
    }

    // Increment scope level
    new_scope->scope = curr_scope->scope + 1;

    // Link up to the current scope
    new_scope->prev = curr_scope;

    // Make the new scope a 'leaf'
    new_scope->next = NULL;

    // Link down to the new scope
    curr_scope->next = new_scope;

    debug("Entering scope %d from scope %d", new_scope->scope, curr_scope->scope);

    // The new scope becomes the current scope
    curr_scope = new_scope;

    print_symbol_table(symbol_table);
}

// Return to the parent scope
static void leave_curr_scope(void) {
    symtab_t *old_scope = curr_scope;

    // Ensure this is a leaf scope
    if (NULL != curr_scope->next) {
        log_error(
            "Current scope (level=%d) is not a leaf node. Unable to leave a non-terminal scope",
            curr_scope->scope);
    }

    curr_scope       = curr_scope->prev;
    curr_scope->next = NULL;

    debug("Leaving scope %d and returning to scope %d", old_scope->scope, curr_scope->scope);

    ht_free(&old_scope->table);

    print_symbol_table(symbol_table);
}

void typecheck(node *ast) {
    if (ast != NULL) {

        // Pass 1: Build symbol table
        symbol_table = symtab_new();

        if (symbol_table == NULL) {
            log_error("Unable to allocate symbol table within typechecker");
        }

        debug("Allocated symbol table for global scope (scope=%d)", symbol_table->scope);
        curr_scope = symbol_table;

        do_typecheck(ast);
    }
}

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

static bool is_numerical_type(type_t type) {
    return (type.datatype == D_FLOAT || type.datatype == D_INTEGER);
}

// Use for leaf nodes
static type_t get_type(node *n) {
    if (NULL == n) {
        log_error("Unable to access node for type checking");
    }

    type_t type = {0};

    switch (n->type) {
        case N_IDENT:
            // Get identifier type from the symbol table
            binding_t *ident_binding = symtab_lookup(curr_scope, n->data.identifier.name);
            if (NULL != ident_binding) {
                switch (ident_binding->symbol_type) {
                    case SYMBOL_TYPE_FUNCTION:
                        // type.datatype = ident_binding->data.function_type.return_type;
                        // type.is_function = true;
                        log_error("SYMBOL_TYPE_FUNCTION not implemented yet: %s",
                                  ident_binding->name);
                        break;
                    case SYMBOL_TYPE_VARIABLE:
                        type.datatype = ident_binding->data.variable_type.type;
                        type.is_array = ident_binding->data.variable_type.is_array_type;
                        break;
                    case SYMBOL_TYPE_STRUCTURE:
                        log_error("SYMBOL_TYPE_STRUCTURE not implemented yet: %s",
                                  ident_binding->name);
                        break;
                    case SYMBOL_TYPE_MEMBER:
                        log_error("SYMBOL_TYPE_MEMBER not implemented yet: %s",
                                  ident_binding->name);
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
        case N_NIL:
            type.datatype = D_NIL;
            break;
        case N_BINOP_EXPR:
            // Visit the LHS in case this is nested
            do_typecheck(n->data.bin_op_expr.lhs);
            type_t lhs = get_type(n->data.bin_op_expr.lhs);

            // Visit the LHS in case this is nested
            do_typecheck(n->data.bin_op_expr.rhs);
            type_t rhs = get_type(n->data.bin_op_expr.rhs);

            switch (n->data.bin_op_expr.operator) {
                case T_PLUS:
                case T_MINUS:
                case T_MUL:
                case T_DIV:
                case T_MOD:
                    if (is_numerical_type(lhs) || is_numerical_type(rhs)) {
                        if (lhs.datatype == D_FLOAT || rhs.datatype == D_FLOAT) {
                            type.datatype = D_FLOAT;
                        } else {
                            type.datatype = D_INTEGER;
                        }
                    }
                    break;
                case T_LT:
                case T_GT:
                case T_EQ:
                case T_LE:
                case T_GE:
                case T_NE:
                case T_AND:
                case T_OR:
                case T_BANG:
                    type.datatype = D_BOOLEAN;
                default:
                    // print_ast(n);
                    // log_error("Unsupported operator type %d", n->data.bin_op_expr.operator);
            }
            break;
        default:
            print_ast(n);
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

static void typecheck_block_stmt(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for typechecking", __FUNCTION__);
    }

    vecnode *vn = ast->data.block_stmt.statements->head;
    while (NULL != vn) {
        node *n = vn->data;
        do_typecheck(n);

        vn = vn->next;
    }
}

static void typecheck_var_decl(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for typechecking", __FUNCTION__);
    }

    // Check if binding already exists within current scope
    binding_t *existing_binding = symtab_lookup(curr_scope, ast->data.var_decl.name);
    if (NULL != existing_binding) {
        char err_msg[MAX_ERROR_LEN] = {0};
        snprintf(err_msg, MAX_ERROR_LEN, "Redefinition of '%s'", existing_binding->name);
        type_error(err_msg, ast);
    }

    // Check the initializer
    do_typecheck(ast->data.var_decl.value);
    type_t init_type = get_type(ast->data.var_decl.value);
    if (ast->data.var_decl.type != init_type.datatype) {
        char err_msg[MAX_ERROR_LEN] = {0};
        snprintf(err_msg, MAX_ERROR_LEN,
                 "Type mismatch between variable type and initialization value. Expected '%s'. Got "
                 "'%s'.",
                 type_to_str(ast->data.var_decl.type), type_to_str(init_type.datatype));
        type_error(err_msg, ast);
    }

    // Create new binding
    binding_t *new_binding = mk_binding(SYMBOL_TYPE_VARIABLE);
    if (NULL == new_binding) {
        log_error("%s(): Unable to create new binding_t", __FUNCTION__);
    }

    // Populate binding data
    snprintf(new_binding->name, MAX_LITERAL, ast->data.var_decl.name);
    snprintf(new_binding->data.variable_type.struct_type, MAX_LITERAL,
             ast->data.var_decl.struct_type);

    new_binding->data.variable_type.type           = ast->data.var_decl.type;
    new_binding->data.variable_type.is_array_type  = ast->data.var_decl.is_array;
    new_binding->data.variable_type.is_struct_type = ast->data.var_decl.is_struct;
    new_binding->data.variable_type.num_dimensions = ast->data.var_decl.num_dimensions;

    // Insert binding into symbol table
    symtab_insert(curr_scope, new_binding);
    print_symbol_table(symbol_table);
}

static void typecheck_func_decl(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for typechecking", __FUNCTION__);
    }

    // Check if the function name is already defined. We do not support overloading, for now...
    binding_t *ident_binding = symtab_lookup(curr_scope, ast->data.function_decl.name);
    if (NULL != ident_binding) {
        char err_msg[MAX_ERROR_LEN] = {0};
        snprintf(err_msg, MAX_ERROR_LEN, "Redefinition of '%s'. Function is previously declared",
                 ast->data.function_decl.name);
        type_error(err_msg, ast);
    }

    // Create new binding and add it to the current scope
    binding_t *new_binding = mk_binding(SYMBOL_TYPE_FUNCTION);
    if (NULL == new_binding) {
        log_error("%s(): Unable to create new binding_t", __FUNCTION__);
    }

    // Populate binding data
    snprintf(new_binding->name, MAX_LITERAL, ast->data.function_decl.name);
    snprintf(new_binding->data.function_type.struct_type, MAX_LITERAL,
             ast->data.function_decl.struct_type);

    new_binding->data.function_type.return_type    = ast->data.function_decl.type;
    new_binding->data.function_type.is_array_type  = ast->data.function_decl.is_array;
    new_binding->data.function_type.is_struct_type = ast->data.function_decl.is_struct;
    new_binding->data.function_type.num_dimensions = ast->data.function_decl.num_dimensions;
    new_binding->data.function_type.num_args       = vector_length(ast->data.function_decl.formals);
    new_binding->data.function_type.formals        = ast->data.function_decl.formals;

    // Insert binding into symbol table
    symtab_insert(curr_scope, new_binding);
    print_symbol_table(symbol_table);

    // Now, create a new scope and enter the function body
    enter_new_scope(new_binding->name);

    // Add the formals to the new scope
    vecnode *vn = ast->data.function_decl.formals->head;
    while (NULL != vn) {
        node *n = vn->data;

        do_typecheck(n);

        vn = vn->next;
    }

    // Check the body
    do_typecheck(ast->data.function_decl.body);

    // Leave scope
    leave_curr_scope();
}

static void typecheck_call_expr(node *ast) { assert(false && "Not yet implemented"); }

static void typecheck_formal(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for typechecking", __FUNCTION__);
    }

    // Check if we've already seen this formal within the current scope
    binding_t *formal_binding = symtab_lookup(curr_scope, ast->data.formal.name);
    if (NULL != formal_binding) {
        char err_msg[MAX_ERROR_LEN] = {0};
        snprintf(err_msg, MAX_ERROR_LEN,
                 "Redefinition of '%s'. Function formal argument is previously declared",
                 ast->data.formal.name);
        type_error(err_msg, ast);
    } else {
        // Add it
        binding_t *new_binding = mk_binding(SYMBOL_TYPE_VARIABLE);
        if (NULL == new_binding) {
            log_error("%s(): Unable to create new binding_t", __FUNCTION__);
        }

        // Populate binding data
        snprintf(new_binding->name, MAX_LITERAL, ast->data.formal.name);
        snprintf(new_binding->data.variable_type.struct_type, MAX_LITERAL,
                 ast->data.formal.struct_type);

        new_binding->data.variable_type.type           = ast->data.formal.type;
        new_binding->data.variable_type.is_array_type  = ast->data.formal.is_array;
        new_binding->data.variable_type.is_struct_type = ast->data.formal.is_struct;
        new_binding->data.variable_type.num_dimensions = ast->data.formal.num_dimensions;

        // Insert binding into symbol table
        symtab_insert(curr_scope, new_binding);
        print_symbol_table(symbol_table);
    }
}

static void typecheck_ident(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for typechecking", __FUNCTION__);
    }

    // Check if the identifier is within the symbol table
    // For now, check only global scope
    binding_t *ident_binding = symtab_lookup(curr_scope, ast->data.identifier.name);
    if (NULL == ident_binding) {
        char err_msg[MAX_ERROR_LEN] = {0};
        snprintf(err_msg, MAX_ERROR_LEN, "Undeclared identifier '%s'", ast->data.identifier.name);
        type_error(err_msg, ast);
    }
}

static void typecheck_binop_expr(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for typechecking", __FUNCTION__);
    }

    // Visit LHS
    do_typecheck(ast->data.bin_op_expr.lhs);

    // Visit RHS
    do_typecheck(ast->data.bin_op_expr.rhs);

    type_t lhs = get_type(ast->data.bin_op_expr.lhs);
    type_t rhs = get_type(ast->data.bin_op_expr.rhs);

    switch (ast->data.bin_op_expr.operator) {
        case T_PLUS:
        case T_MINUS:
        case T_MUL:
        case T_DIV:
        case T_MOD:
            if (!is_numerical_type(lhs) || !is_numerical_type(rhs)) {
                // If either datatype is not a number
                debug("got here");

                // And the operator is arithmetic, raise an error
                char err_msg[MAX_ERROR_LEN] = {0};
                snprintf(err_msg, MAX_ERROR_LEN,
                         "Type mismatch. Both data types must be numeric in order to perform "
                         "arithmetic operations. Left-hand side is '%s'. Right hand side is '%s'.",
                         type_to_str(lhs.datatype), type_to_str(rhs.datatype));
                type_error(err_msg, ast);
            }
            break;
        case T_LT:
        case T_GT:
        case T_EQ:
        case T_LE:
        case T_GE:
        case T_NE:
        case T_AND:
        case T_OR:
        case T_BANG:
            if (!match_types(ast->data.bin_op_expr.lhs, ast->data.bin_op_expr.rhs, &lhs, &rhs)) {
                char err_msg[MAX_ERROR_LEN] = {0};
                snprintf(err_msg, MAX_ERROR_LEN,
                         "Type mismatch. Left-hand side is '%s'. Right hand side is '%s'.",
                         type_to_str(lhs.datatype), type_to_str(rhs.datatype));
                type_error(err_msg, ast);
            }

            break;
        default:
            type_error("Unsupported operator for binary expression. Expected arithmetic or logical "
                       "operators",
                       ast);
    }
}

static void typecheck_assign_expr(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for typechecking", __FUNCTION__);
    }

    // Visit LHS
    do_typecheck(ast->data.assign_expr.lhs);

    // Visit RHS
    do_typecheck(ast->data.assign_expr.rhs);

    type_t lhs_type = {0};
    type_t rhs_type = {0};

    // Check if LHS type and RHS type match
    if (!match_types(ast->data.assign_expr.lhs, ast->data.assign_expr.rhs, &lhs_type, &rhs_type)) {
        char err_msg[MAX_ERROR_LEN] = {0};
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

static void typecheck_return_stmt(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for typechecking", __FUNCTION__);
    }

    // Visit the expression
    do_typecheck(ast->data.return_stmt.expr);

    // Get its type
    type_t return_expr_type = get_type(ast->data.return_stmt.expr);

    // Compare against the function return type
    debug("Return expr type is %d", return_expr_type.datatype);

    // Since function definitions are contained within the parent scope, check the parent scope for
    // our function's return type
    binding_t *func_binding = symtab_lookup(curr_scope->prev, curr_scope->name);
    if (NULL != func_binding) {
        if (return_expr_type.datatype != func_binding->data.function_type.return_type) {
            char err_msg[MAX_ERROR_LEN] = {0};
            snprintf(err_msg, MAX_ERROR_LEN,
                     "Type mismatch between '%s' return type and return statement. Expected '%s'. "
                     "Got '%s'.",
                     func_binding->name, type_to_str(func_binding->data.function_type.return_type),
                     type_to_str(return_expr_type.datatype));
            type_error(err_msg, ast);
        }
    }
}

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
