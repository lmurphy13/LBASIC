#if defined(HAS_TRANSLATOR)

/**
 * https://youtu.be/xtouovp9kvQ?si=9HyFSgZCTtvTRpqM
 */

#include "translate.h"

#include "assert.h"
#include "error.h"
#include "typechecker.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static vector *ir_list        = NULL;
static symtab_t *symbol_table = NULL;

static unsigned int label_count = 0;
static unsigned int temp_count  = 0;

static void translate_program(node *ast, symtab_t *symtab);
static void translate_block_stmt(node *ast, symtab_t *symtab);
static void translate_var_decl(node *ast, symtab_t *symtab);
static void translate_call_expr(node *ast, symtab_t *symtab);
static void translate_func_decl(node *ast, symtab_t *symtab);
static void translate_formal(node *ast, symtab_t *symtab);
static void translate_ident(node *ast, symtab_t *symtab);
static void translate_binop_expr(node *ast, symtab_t *symtab);
static void translate_assign_expr(node *ast, symtab_t *symtab);
static void translate_if_stmt(node *ast, symtab_t *symtab);
static void translate_literal(node *ast, symtab_t *symtab);
static void translate_return_stmt(node *ast, symtab_t *symtab);
static void translate_nil(node *ast, symtab_t *symtab);
static void translate_struct_decl(node *ast, symtab_t *symtab);
static void translate_member_decl(node *ast, symtab_t *symtab);
static void translate_struct_access(node *ast, symtab_t *symtab);
static void translate_label_decl(node *ast, symtab_t *symtab);
static void translate_goto_stmt(node *ast, symtab_t *symtab);
static void translate_array_init_expr(node *ast, symtab_t *symtab);
static void translate_array_access_expr(node *ast, symtab_t *symtab);
static void translate_while_stmt(node *ast, symtab_t *symtab);
static void translate_empty_expr(node *ast, symtab_t *symtab);
static void translate_neg_expr(node *ast, symtab_t *symtab);
static void translate_not_expr(node *ast, symtab_t *symtab);

static ir_node *mk_ir_node(ir_type type);
static void print_ir(vector *ir);
static void get_temp(char *input);
static void get_label(char *input);

static ir_node *mk_ir_node(ir_type type) {
    ir_node *new_node = (ir_node *)calloc(1, sizeof(ir_node));

    if (NULL != new_node) {
        new_node->type         = type;
        new_node->arg1_binding = NULL;
        new_node->arg2_binding = NULL;
        new_node->arg3_binding = NULL;
    }

    return new_node;
}

static void add_comment(ir_node *node, char *comment) {
    if (NULL != node && NULL != comment) {
        snprintf(node->comment, MAX_COMMENT, "\t\t// %s", comment);
    }
}

static void get_temp(char *input) {
    if (NULL != input) {
        sprintf(input, "t%d", temp_count++);
    }
}

static void get_label(char *input) {
    if (NULL != input) {
        sprintf(input, ".L%d", label_count++);
    }
}

static void PUSH(char *arg, char *comment) {
    if (NULL == arg) {
        log_error("%s(): Unable to access arg", __FUNCTION__);
    }

    ir_node *push_node = mk_ir_node(IR_PUSH);
    if (NULL == push_node) {
        log_error("%s(): Unable to create PUSH node", __FUNCTION__);
    }

    snprintf(push_node->arg1, MAX_ARGUMENT, arg);

    if (NULL != comment) {
        add_comment(push_node, comment);
    }
    vector_add(ir_list, push_node);
}

static void POP(char *arg, char *comment) {
    if (NULL == arg) {
        log_error("%s(): Unable to access arg", __FUNCTION__);
    }

    ir_node *pop_node = mk_ir_node(IR_POP);
    if (NULL == pop_node) {
        log_error("%s(): Unable to create POP node", __FUNCTION__);
    }

    snprintf(pop_node->arg1, MAX_ARGUMENT, arg);

    if (NULL != comment) {
        add_comment(pop_node, comment);
    }
    vector_add(ir_list, pop_node);
}

static ir_node *LOAD(char *arg1, char *arg2, char *comment) {
    if (NULL == arg1) {
        log_error("%s(): Unable to access arg1", __FUNCTION__);
    }

    if (NULL == arg2) {
        log_error("%s(): Unable to access arg2", __FUNCTION__);
    }

    ir_node *node = mk_ir_node(IR_LOAD);
    if (NULL == node) {
        log_error("%s(): Unable to create LOAD node", __FUNCTION__);
    }

    snprintf(node->arg1, MAX_ARGUMENT, arg1);
    snprintf(node->arg2, MAX_ARGUMENT, arg2);

    if (NULL != comment) {
        add_comment(node, comment);
    }
    vector_add(ir_list, node);

    return node;
}

static ir_node *STORE(char *arg1, char *arg2, char *comment) {
    if (NULL == arg1) {
        log_error("%s(): Unable to access arg1", __FUNCTION__);
    }

    if (NULL == arg2) {
        log_error("%s(): Unable to access arg2", __FUNCTION__);
    }

    ir_node *node = mk_ir_node(IR_STORE);
    if (NULL == node) {
        log_error("%s(): Unable to create STORE node", __FUNCTION__);
    }

    snprintf(node->arg1, MAX_ARGUMENT, arg1);
    snprintf(node->arg2, MAX_ARGUMENT, arg2);

    if (NULL != comment) {
        add_comment(node, comment);
    }
    vector_add(ir_list, node);

    return node;
}
// DST = SOURCE1 + SOURCE2
static ir_node *ADD(char *arg1, char *arg2, char *arg3, char *comment) {
    if (NULL == arg1) {
        log_error("%s(): Unable to access arg1", __FUNCTION__);
    }

    if (NULL == arg2) {
        log_error("%s(): Unable to access arg2", __FUNCTION__);
    }

    if (NULL == arg3) {
        log_error("%s(): Unable to access arg3", __FUNCTION__);
    }

    ir_node *node = mk_ir_node(IR_ADD);
    if (NULL == node) {
        log_error("%s(): Unable to create ADD node", __FUNCTION__);
    }

    snprintf(node->arg1, MAX_ARGUMENT, arg1);
    snprintf(node->arg2, MAX_ARGUMENT, arg2);
    snprintf(node->arg3, MAX_ARGUMENT, arg3);

    if (NULL != comment) {
        add_comment(node, comment);
    }
    vector_add(ir_list, node);

    return node;
}

static ir_node *SUB(char *arg1, char *arg2, char *arg3, char *comment) {
    if (NULL == arg1) {
        log_error("%s(): Unable to access arg1", __FUNCTION__);
    }

    if (NULL == arg2) {
        log_error("%s(): Unable to access arg2", __FUNCTION__);
    }

    if (NULL == arg3) {
        log_error("%s(): Unable to access arg3", __FUNCTION__);
    }

    ir_node *node = mk_ir_node(IR_SUB);
    if (NULL == node) {
        log_error("%s(): Unable to create SUB node", __FUNCTION__);
    }

    snprintf(node->arg1, MAX_ARGUMENT, arg1);
    snprintf(node->arg2, MAX_ARGUMENT, arg2);
    snprintf(node->arg3, MAX_ARGUMENT, arg3);

    if (NULL != comment) {
        add_comment(node, comment);
    }
    vector_add(ir_list, node);

    return node;
}

static ir_node *MUL(char *arg1, char *arg2, char *arg3, char *comment) {
    if (NULL == arg1) {
        log_error("%s(): Unable to access arg1", __FUNCTION__);
    }

    if (NULL == arg2) {
        log_error("%s(): Unable to access arg2", __FUNCTION__);
    }

    if (NULL == arg3) {
        log_error("%s(): Unable to access arg3", __FUNCTION__);
    }

    ir_node *node = mk_ir_node(IR_MUL);
    if (NULL == node) {
        log_error("%s(): Unable to create STORE node", __FUNCTION__);
    }

    snprintf(node->arg1, MAX_ARGUMENT, arg1);
    snprintf(node->arg2, MAX_ARGUMENT, arg2);
    snprintf(node->arg3, MAX_ARGUMENT, arg3);

    if (NULL != comment) {
        add_comment(node, comment);
    }
    vector_add(ir_list, node);

    return node;
}

static ir_node *DIV(char *arg1, char *arg2, char *arg3, char *comment) {
    if (NULL == arg1) {
        log_error("%s(): Unable to access arg1", __FUNCTION__);
    }

    if (NULL == arg2) {
        log_error("%s(): Unable to access arg2", __FUNCTION__);
    }

    if (NULL == arg3) {
        log_error("%s(): Unable to access arg3", __FUNCTION__);
    }

    ir_node *node = mk_ir_node(IR_DIV);
    if (NULL == node) {
        log_error("%s(): Unable to create DIV node", __FUNCTION__);
    }

    snprintf(node->arg1, MAX_ARGUMENT, arg1);
    snprintf(node->arg2, MAX_ARGUMENT, arg2);
    snprintf(node->arg3, MAX_ARGUMENT, arg3);

    if (NULL != comment) {
        add_comment(node, comment);
    }
    vector_add(ir_list, node);

    return node;
}

static ir_node *CALL(char *arg, char *comment) {
    if (NULL == arg) {
        log_error("%s(): Unable to access arg", __FUNCTION__);
    }

    ir_node *node = mk_ir_node(IR_CALL);
    if (NULL == node) {
        log_error("%s(): Unable to create CALL node", __FUNCTION__);
    }

    snprintf(node->arg1, MAX_ARGUMENT, arg);

    if (NULL != comment) {
        add_comment(node, comment);
    }
    vector_add(ir_list, node);

    return node;
}

static ir_node *JUMP(char *arg, char *comment) {
    if (NULL == arg) {
        log_error("%s(): Unable to access arg", __FUNCTION__);
    }

    ir_node *node = mk_ir_node(IR_JUMP);
    if (NULL == node) {
        log_error("%s(): Unable to create JUMP node", __FUNCTION__);
    }

    snprintf(node->arg1, MAX_ARGUMENT, arg);

    if (NULL != comment) {
        add_comment(node, comment);
    }
    vector_add(ir_list, node);

    return node;
}

static ir_node *CJUMP(char *arg2, char *arg3, char *if_true, char *if_false,
                      token_type rel_operator, char *comment) {
    // if (NULL == arg1) {
    //     log_error("%s(): Unable to access arg1", __FUNCTION__);
    // }

    if (NULL == arg2) {
        log_error("%s(): Unable to access arg2", __FUNCTION__);
    }

    if (NULL == arg3) {
        log_error("%s(): Unable to access arg3", __FUNCTION__);
    }

    if (NULL == if_true) {
        log_error("%s(): Unable to access if_true", __FUNCTION__);
    }

    if (NULL == if_false) {
        log_error("%s(): Unable to access if_false", __FUNCTION__);
    }

    ir_node *node = mk_ir_node(IR_CJUMP);
    if (NULL == node) {
        log_error("%s(): Unable to create CALL node", __FUNCTION__);
    }

    // snprintf(node->arg1, MAX_ARGUMENT, arg1);
    snprintf(node->arg2, MAX_ARGUMENT, arg2);
    snprintf(node->arg3, MAX_ARGUMENT, arg3);
    snprintf(node->label_if_true, MAX_ARGUMENT, if_true);
    snprintf(node->label_if_false, MAX_ARGUMENT, if_false);
    node->rel_operator = rel_operator;

    if (NULL != comment) {
        add_comment(node, comment);
    }
    vector_add(ir_list, node);

    return node;
}

ir_node *LABEL(char *arg, char *comment) {
    if (NULL == arg) {
        log_error("%s(): Unable to access arg", __FUNCTION__);
    }

    ir_node *node = mk_ir_node(IR_LABEL);
    if (NULL == node) {
        log_error("%s(): Unable to create LABEL node", __FUNCTION__);
    }

    snprintf(node->arg1, MAX_ARGUMENT, arg);

    if (NULL != comment) {
        add_comment(node, comment);
    }
    vector_add(ir_list, node);

    return node;
}

static ir_node *RETURN(char *comment) {
    ir_node *node = mk_ir_node(IR_RETURN);
    if (NULL == node) {
        log_error("%s(): Unabel to create RETURN node", __FUNCTION__);
    }

    if (NULL != comment) {
        add_comment(node, comment);
    }
    vector_add(ir_list, node);

    return node;
}

vector *translate_init(node *ast) {
    if (NULL != ast) {
        ir_list = mk_vector();

        if (NULL == ir_list) {
            log_error("Unable to allocate vector for IR list");
        }

        // Get a reference to the symbol table
        symbol_table = get_symbol_table();
        if (NULL == symbol_table) {
            log_error("%s(): Unable to access symbol table", __FUNCTION__);
        }
    }

    return ir_list;
}

void do_translate(node *ast, symtab_t *symtab) {
    if (NULL == ast) {
        log_error("Unable to access node for translation");
    }

    switch (ast->type) {
        case N_PROGRAM:
            translate_program(ast, symtab);
            break;
        case N_BLOCK_STMT:
            translate_block_stmt(ast, symtab);
            break;
        case N_VAR_DECL:
            translate_var_decl(ast, symtab);
            break;
        case N_FUNC_DECL:
            translate_func_decl(ast, symtab);
            break;
        case N_CALL_EXPR:
            translate_call_expr(ast, symtab);
            break;
        case N_FORMAL:
            translate_formal(ast, symtab);
            break;
        case N_IDENT:
            translate_ident(ast, symtab);
            break;
        case N_BINOP_EXPR:
            translate_binop_expr(ast, symtab);
            break;
        case N_ASSIGN_EXPR:
            translate_assign_expr(ast, symtab);
            break;
        case N_IF_STMT:
            translate_if_stmt(ast, symtab);
            break;
        case N_INTEGER_LITERAL:
        case N_FLOAT_LITERAL:
        case N_STRING_LITERAL:
        case N_BOOL_LITERAL:
            translate_literal(ast, symtab);
            break;
        case N_RETURN_STMT:
            translate_return_stmt(ast, symtab);
            break;
        case N_NIL:
            translate_nil(ast, symtab);
            break;
        case N_STRUCT_DECL:
            translate_struct_decl(ast, symtab);
            break;
        case N_MEMBER_DECL:
            translate_member_decl(ast, symtab);
            break;
        case N_STRUCT_ACCESS_EXPR:
            translate_struct_access(ast, symtab);
            break;
        case N_LABEL_DECL:
            translate_label_decl(ast, symtab);
            break;
        case N_GOTO_STMT:
            translate_goto_stmt(ast, symtab);
            break;
        case N_ARRAY_INIT_EXPR:
            translate_array_init_expr(ast, symtab);
            break;
        case N_ARRAY_ACCESS_EXPR:
            translate_array_access_expr(ast, symtab);
            break;
        case N_WHILE_STMT:
            translate_while_stmt(ast, symtab);
            break;
        case N_EMPTY_EXPR:
            translate_empty_expr(ast, symtab);
            break;
        case N_NEG_EXPR:
            translate_neg_expr(ast, symtab);
            break;
        case N_NOT_EXPR:
            translate_not_expr(ast, symtab);
            break;
        default:
            log_error("Unknown node type", ast);
            break;
    }
}

static void translate_program(node *ast, symtab_t *symtab) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for translation", __FUNCTION__);
    }

    if (NULL == ast->data.program.statements->head) {
        log_error("%s(): Unable to access statement list for translation", __FUNCTION__);
    }

    vecnode *vn = ast->data.program.statements->head;
    while (NULL != vn) {
        node *n = vn->data;
        if (NULL != n) {
            do_translate(n, symtab);
            vn = vn->next;
        }
    }

    print_ir(ir_list);
}

static void translate_block_stmt(node *ast, symtab_t *symtab) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for translation", __FUNCTION__);
    }

    if (NULL == ast->data.block_stmt.statements->head) {
        log_error("%s(): Unable to access block statement list for translation", __FUNCTION__);
    }

    symbol_table = get_symbol_table();

    vecnode *vn = ast->data.block_stmt.statements->head;
    while (NULL != vn) {
        node *n = vn->data;
        if (NULL != n) {
            do_translate(n, symtab);
            vn = vn->next;
        }
    }

    print_ir(ir_list);
}

// TYPE var := Node;
static void translate_var_decl(node *ast, symtab_t *symtab) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for translation", __FUNCTION__);
    }

    char label[MAX_ARGUMENT] = {0};
    char tmp1[MAX_ARGUMENT]  = {0};
    char tmp2[MAX_ARGUMENT]  = {0};

    // Reset scope to current
    symbol_table = get_symbol_table();

    ir_node *load_node      = NULL;
    binding_t *decl_binding = symtab_lookup(symbol_table, ast->data.var_decl.name, false);
    if (NULL == decl_binding) {
        log_error("Cannot access variable declaration binding");
    }

    // Get RHS
    // do_translate(ast->data.var_decl.value);
    switch (ast->data.var_decl.value->type) {
        case N_INTEGER_LITERAL:
            char int_val[MAX_ARGUMENT] = {0};
            get_temp(tmp1);
            snprintf(int_val, MAX_ARGUMENT, "$%d",
                     ast->data.var_decl.value->data.integer_literal.value);
            snprintf(decl_binding->temp, MAX_ARGUMENT, tmp1);
            load_node               = LOAD(tmp1, int_val, ast->data.var_decl.name);
            load_node->arg1_binding = decl_binding;
            break;
        case N_STRING_LITERAL:
            get_label(label);
            snprintf(decl_binding->temp, MAX_ARGUMENT, label);
            ir_node *store_node = STORE(label, ast->data.var_decl.value->data.string_literal.value,
                                        ast->data.var_decl.name);
            store_node->arg1_binding = decl_binding;
            break;
        case N_BOOL_LITERAL:
            char bool_val[MAX_ARGUMENT] = {0};
            get_temp(tmp1);
            snprintf(bool_val, MAX_ARGUMENT, "$%d",
                     ast->data.var_decl.value->data.bool_literal.value);
            snprintf(decl_binding->temp, MAX_ARGUMENT, tmp1);
            load_node               = LOAD(tmp1, bool_val, ast->data.var_decl.name);
            load_node->arg1_binding = decl_binding;
            break;
        case N_IDENT:
            binding_t *ident_binding =
                symtab_lookup(symbol_table, ast->data.var_decl.value->data.identifier.name, false);
            if (NULL != ident_binding) {
                get_temp(tmp1);
                snprintf(ident_binding->temp, MAX_ARGUMENT, tmp1);
                load_node               = LOAD(tmp1, ident_binding->name, ast->data.var_decl.name);
                load_node->arg2_binding = ident_binding;
                load_node->arg1_binding = decl_binding;
            }
            break;
        case N_BINOP_EXPR:
            binding_t *var_binding = symtab_lookup(symbol_table, ast->data.var_decl.name, false);
            if (NULL != var_binding) {
                // Result is in a temporary
                do_translate(ast->data.var_decl.value, symtab);
                // sprintf(tmp1, "t%d", temp_count-1);   // Previously written temp
                //  Get most recent temporary
                ir_node *last = (ir_node *)ir_list->tail->data;
                get_temp(tmp1);

                // Is it a boolean expression?
                if ((last->rel_operator >= T_LT) && (last->rel_operator <= T_OR)) {
                    // Get exit label
                    get_label(label);

                    // Emit if true label
                    LABEL(last->label_if_true, "true");

                    // Do assigment
                    LOAD(tmp1, "$1", ast->data.var_decl.name);

                    // Jump to exit
                    JUMP(label, NULL);

                    // Emit if false label
                    LABEL(last->label_if_false, "false");

                    // Do assignment
                    LOAD(tmp1, "$0", ast->data.var_decl.name);

                    // Emit exit
                    LABEL(label, "exit");
                } else {
                    snprintf(tmp2, MAX_ARGUMENT, last->arg1);

                    load_node = LOAD(tmp1, tmp2, ast->data.var_decl.name);
                    snprintf(var_binding->temp, MAX_ARGUMENT, tmp1);
                    load_node->arg1_binding = var_binding;
                }
            }

            break;
        case N_FLOAT_LITERAL:
        default:
            log_error("%s(): Value type %d not yet implemented", __FUNCTION__,
                      ast->data.var_decl.value->type);
    }

    print_ir(ir_list);
}

static void translate_call_expr(node *ast, symtab_t *symtab) { assert(false && "Not implemented yet"); }

static void translate_func_decl(node *ast, symtab_t *symtab) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for translation", __FUNCTION__);
    }

    // Do nothing

   // LABEL(ast->data.function_decl.name, "function entry");

    // symbol_table = get_symbol_table();

    // binding_t *func_binding = symtab_lookup(symbol_table, ast->data.function_decl.name, false);

    // LABEL(ast->data.function_decl.name, "function entry");

    // if (func_binding->data.function_type.num_args > 0) {

    //     symbol_table = get_symbol_table();
    //     debug("name: %s", symbol_table->name);
    //     debug("level: %d", symbol_table->level);

    //     vecnode *vn = ast->data.function_decl.formals->head;
    //     while (NULL != vn) {
    //         node *n = vn->data;

    //         do_translate(n);

    //         vn = vn->next;
    //     }
    // }


    // if (NULL != ast->data.function_decl.body) {
    //     do_translate(ast->data.function_decl.body);
    // }

    // print_ir(ir_list);
}

static void translate_formal(node *ast, symtab_t *symtab) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for translation", __FUNCTION__);
    }

    char tmp1[MAX_ARGUMENT] = {0};

    debug("here in %s()", __FUNCTION__);

    symbol_table = get_symbol_table();

    binding_t *formal_binding = symtab_lookup(symbol_table, ast->data.formal.name, true);
    if (NULL != formal_binding) {
        get_temp(tmp1);
        snprintf(formal_binding->temp, MAX_ARGUMENT, tmp1);
        ir_node *load_node               = LOAD(tmp1, formal_binding->name, ast->data.formal.name);
        load_node->arg1_binding = formal_binding;
    }
}

static void translate_ident(node *ast, symtab_t *symtab) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for translation", __FUNCTION__);
    }
    // Do nothing
}

static void translate_binop_expr(node *ast, symtab_t *symtab) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for translation", __FUNCTION__);
    }

    node *lhs               = ast->data.bin_op_expr.lhs;
    node *rhs               = ast->data.bin_op_expr.rhs;
    char tmp1[MAX_ARGUMENT] = {0};
    char tmp2[MAX_ARGUMENT] = {0};
    char tmp3[MAX_ARGUMENT] = {0};
    binding_t *lhs_binding  = NULL;
    binding_t *rhs_binding  = NULL;

    // Reset scope to current
    symbol_table = get_symbol_table();

    do_translate(lhs, symtab);
    do_translate(rhs, symtab);

    if (lhs->type == N_IDENT) {
        lhs_binding = symtab_lookup(symbol_table, lhs->data.identifier.name, false);
        if (NULL != lhs_binding) {
            snprintf(tmp2, MAX_ARGUMENT, lhs_binding->temp);
        }
    } else if (lhs->type == N_INTEGER_LITERAL) {
        snprintf(tmp2, MAX_ARGUMENT, "$%d", lhs->data.integer_literal.value);
    } else if (lhs->type == N_FLOAT_LITERAL) {
        snprintf(tmp2, MAX_ARGUMENT, "$%f", lhs->data.float_literal.value);
        log_error("%s(): No float support yet", __FUNCTION__);
    } else if (lhs->type == N_BINOP_EXPR) {
        //do_translate(lhs);
        debug("here lhs");
    } else {
        log_error("%s(): LHS type %d not supported yet", __FUNCTION__, lhs->type);
    }

    if (rhs->type == N_IDENT) {
        rhs_binding = symtab_lookup(symbol_table, rhs->data.identifier.name, false);
        snprintf(tmp3, MAX_ARGUMENT, rhs_binding->temp);
    } else if (rhs->type == N_INTEGER_LITERAL) {
        snprintf(tmp3, MAX_ARGUMENT, "$%d", rhs->data.integer_literal.value);
    } else if (rhs->type == N_FLOAT_LITERAL) {
        snprintf(tmp3, MAX_ARGUMENT, "$%f", rhs->data.float_literal.value);
        log_error("%s(): No float support yet", __FUNCTION__);
    } else if (rhs->type == N_BINOP_EXPR) {
        //do_translate(lhs);
        debug("here rhs");
    } else {
        log_error("%s(): RHS type %d not supported yet", __FUNCTION__, rhs->type);
    }

    switch (ast->data.bin_op_expr.operator) {
        case T_PLUS:
            get_temp(tmp1);
            ir_node *add_node      = ADD(tmp1, tmp2, tmp3, NULL);
            add_node->arg2_binding = lhs_binding;
            add_node->arg3_binding = rhs_binding;
            break;
        case T_MINUS:
            get_temp(tmp1);
            ir_node *sub_node      = SUB(tmp1, tmp2, tmp3, NULL);
            sub_node->arg2_binding = lhs_binding;
            sub_node->arg3_binding = rhs_binding;
            break;
        case T_MUL:
            get_temp(tmp1);
            ir_node *mul_node      = MUL(tmp1, tmp2, tmp3, NULL);
            mul_node->arg2_binding = lhs_binding;
            mul_node->arg3_binding = rhs_binding;
            break;
        case T_DIV:
            get_temp(tmp1);
            ir_node *div_node      = DIV(tmp1, tmp2, tmp3, NULL);
            div_node->arg2_binding = lhs_binding;
            div_node->arg3_binding = rhs_binding;
            break;
        case T_LT:
        case T_GT:
        case T_EQ:
        case T_LE:
        case T_GE:
        case T_NE:
        case T_AND:
        case T_OR:
            // Conditional Jump
            char lbl_if_true[MAX_ARGUMENT]  = {0};
            char lbl_if_false[MAX_ARGUMENT] = {0};
            get_label(lbl_if_true);
            get_label(lbl_if_false);
            CJUMP(tmp2, tmp3, lbl_if_true, lbl_if_false, ast->data.bin_op_expr.operator, NULL);
            break;
        default:
            log_error("Operator %d not supported yet", ast->data.bin_op_expr.operator);
    }

    print_ir(ir_list);
}

static void translate_assign_expr(node *ast, symtab_t *symtab) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for translation", __FUNCTION__);
    }

    node *lhs               = ast->data.assign_expr.lhs;
    node *rhs               = ast->data.assign_expr.rhs;
    char tmp1[MAX_ARGUMENT] = {0};
    char tmp2[MAX_ARGUMENT] = {0};
    binding_t *lhs_binding  = NULL;
    binding_t *rhs_binding  = NULL;
    ir_node *load_node      = NULL;

    // // Reset scope to current
    // // symbol_table = get_symbol_table();

    do_translate(lhs, symtab);
    do_translate(rhs, symtab);

    if (lhs->type == N_IDENT) {
        lhs_binding = symtab_lookup(symbol_table, lhs->data.identifier.name, false);
        snprintf(tmp1, MAX_ARGUMENT, lhs_binding->temp);
    } else {
        log_error("%s(): LHS type %d not supported yet", __FUNCTION__, lhs->type);
    }

    if (rhs->type == N_IDENT) {
        rhs_binding = symtab_lookup(symbol_table, rhs->data.identifier.name, false);
        snprintf(tmp2, MAX_ARGUMENT, rhs_binding->temp);
        load_node = LOAD(tmp1, tmp2, lhs_binding->name);
    } else if (rhs->type == N_INTEGER_LITERAL) {
        snprintf(tmp2, MAX_ARGUMENT, "$%d", rhs->data.integer_literal.value);
        load_node = LOAD(tmp1, tmp2, lhs_binding->name);
    } else if (rhs->type == N_FLOAT_LITERAL) {
        snprintf(tmp2, MAX_ARGUMENT, "$%f", rhs->data.float_literal.value);
        log_error("%s(): No float support yet", __FUNCTION__);
    } else if (rhs->type == N_BINOP_EXPR) {
        //  Get most recent temporary, which should be the resulf of the binop expression
        ir_node *last = (ir_node *)ir_list->tail->data;
        snprintf(tmp2, MAX_ARGUMENT, last->arg1);
        // get_temp(tmp1);
        load_node = LOAD(tmp1, tmp2, lhs_binding->name);
        snprintf(lhs_binding->temp, MAX_ARGUMENT, tmp1);
        load_node->arg1_binding = lhs_binding;
    } else {
        log_error("%s(): RHS type %d not supported yet", __FUNCTION__, rhs->type);
    }
}

static void translate_if_stmt(node *ast, symtab_t *symtab) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for translation", __FUNCTION__);
    }

    char exit_label[MAX_ARGUMENT] = {0};

    // Translate test
    do_translate(ast->data.if_stmt.test, symtab);

    // Get last CJUMP
    ir_node *cjump = (ir_node *)ir_list->tail->data;
    LABEL(cjump->label_if_true, "if true");

    get_label(exit_label);

    // Translate If body
    do_translate(ast->data.if_stmt.body, symtab);
    JUMP(exit_label, NULL);

    // Translate Else body
    LABEL(cjump->label_if_false, "if false");
    if (NULL != ast->data.if_stmt.else_stmt) {
        do_translate(ast->data.if_stmt.else_stmt, symtab);
        // Don't need to jump to exit, since we can fall-through to exit_label
    }

    LABEL(exit_label, "exit");
    print_ir(ir_list);
}

static void translate_literal(node *ast, symtab_t *symtab) {
    // Do nothing
    debug("Looking at literal");
    print_node(ast, 0);
}

static void translate_return_stmt(node *ast, symtab_t *symtab) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for translation", __FUNCTION__);
    }

    RETURN(NULL);
}

static void translate_nil(node *ast, symtab_t *symtab) { assert(false && "Not implemented yet"); }

static void translate_struct_decl(node *ast, symtab_t *symtab) { assert(false && "Not implemented yet"); }

static void translate_member_decl(node *ast, symtab_t *symtab) { assert(false && "Not implemented yet"); }

static void translate_struct_access(node *ast, symtab_t *symtab) { assert(false && "Not implemented yet"); }

static void translate_label_decl(node *ast, symtab_t *symtab) { assert(false && "Not implemented yet"); }

static void translate_goto_stmt(node *ast, symtab_t *symtab) { assert(false && "Not implemented yet"); }

static void translate_array_init_expr(node *ast, symtab_t *symtab) { assert(false && "Not implemented yet"); }

static void translate_array_access_expr(node *ast, symtab_t *symtab) { assert(false && "Not implemented yet"); }

static void translate_while_stmt(node *ast, symtab_t *symtab) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for translation", __FUNCTION__);
    }

    char loop_label[MAX_ARGUMENT] = {0};
    char exit_label[MAX_ARGUMENT] = {0};

    get_label(loop_label);
    LABEL(loop_label, "while loop entry");

    // Translate test
    do_translate(ast->data.while_stmt.test, symtab);

    // Get last CJUMP
    ir_node *cjump = (ir_node *)ir_list->tail->data;
    LABEL(cjump->label_if_true, "loop if true");

    //get_label(exit_label);

    if (NULL != ast->data.while_stmt.body) {
        // Translate while body
        do_translate(ast->data.while_stmt.body, symtab);
        JUMP(loop_label, "return to loop entry");
    }

    LABEL(cjump->label_if_false, "loop if false");

    print_ir(ir_list);
}

static void translate_empty_expr(node *ast, symtab_t *symtab) { assert(false && "Not implemented yet"); }

static void translate_neg_expr(node *ast, symtab_t *symtab) { assert(false && "Not implemented yet"); }

static void translate_not_expr(node *ast, symtab_t *symtab) { assert(false && "Not implemented yet"); }

static void print_ir(vector *ir) {
    printf("IR List:\n");
    if (NULL != ir) {
        vecnode *vn_iter = ir->head;
        while (NULL != vn_iter) {
            ir_node *node = (ir_node *)vn_iter->data;

            const char *const arg1 = (strlen(node->arg1) > 0) ? node->arg1 : "";
            const char *const arg2 = (strlen(node->arg2) > 0) ? node->arg2 : "";
            const char *const arg3 = (strlen(node->arg3) > 0) ? node->arg3 : "";
            const char *const label_if_true =
                (strlen(node->label_if_true) > 0) ? node->label_if_true : "";
            const char *const label_if_false =
                (strlen(node->label_if_false) > 0) ? node->label_if_false : "";
            const char *const comment = (strlen(node->comment) > 0) ? node->comment : "";

            switch (node->type) {
                case IR_PUSH:
                    printf("PUSH   %s              %s\n", arg1, comment);
                    break;
                case IR_POP:
                    printf("POP    %s              %s\n", arg1, comment);
                    break;
                case IR_LOAD: //  TMP  (VAL | TMP)
                    printf("LOAD   %s = %s         %s\n", arg1, arg2, comment);
                    break;
                case IR_STORE: // MEM = (VAL | TMP)
                    printf("STORE  %s = %s         %s\n", arg1, arg2, comment);
                    break;
                case IR_ADD: //  TMP = (VAL | TMP) + (VAL | TMP)
                    printf("ADD    %s = %s + %s    %s\n", arg1, arg2, arg3, comment);
                    break;
                case IR_SUB: //  TMP = (VAL | TMP) + (VAL | TMP)
                    printf("SUB    %s = %s - %s    %s\n", arg1, arg2, arg3, comment);
                    break;
                case IR_MUL: //  TMP = (VAL | TMP) + (VAL | TMP)
                    printf("MUL    %s = %s * %s    %s\n", arg1, arg2, arg3, comment);
                    break;
                case IR_DIV: //  TMP = (VAL | TMP) + (VAL | TMP)
                    printf("DIV    %s = %s / %s    %s\n", arg1, arg2, arg3, comment);
                    break;
                case IR_CALL: // FUNC
                    printf("CALL   %s              %s", arg1, comment);
                    break;
                case IR_JUMP: //  LBL
                    printf("JUMP   %s              %s\n", arg1, comment);
                    break;
                case IR_CJUMP:
                    printf("CJUMP  %s  Op: %d  %s  JUMP IfTrue: %s  JUMP IfFalse: %s\n", arg2,
                           node->rel_operator, arg3, label_if_true, label_if_false);
                    break;
                case IR_LABEL: // LBL
                    printf("LABEL  %s              %s\n", arg1, comment);
                    break;
                case IR_RETURN:
                    printf("RETURN                 %s\n", comment);
                    break;
                default:
                    log_error("Unknown node type %d", node->type);
            }

            vn_iter = vn_iter->next;
        }
    }
    printf("End list\n\n");
}

#endif // HAS_TRANSLATOR