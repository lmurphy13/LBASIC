#include "translate.h"

#include "assert.h"
#include "error.h"
#include "symtab.h"
#include "typechecker.h"

static vector *ir_list = NULL;

static void translate_program(node *ast);
static void translate_block_stmt(node *ast);
static void translate_var_decl(node *ast);
static void translate_call_expr(node *ast);
static void translate_func_decl(node *ast);
static void translate_formal(node *ast);
static void translate_ident(node *ast);
static void translate_binop_expr(node *ast);
static void translate_assign_expr(node *ast);
static void translate_if_stmt(node *ast);
static void translate_literal(node *ast);
static void translate_return_stmt(node *ast);
static void translate_nil(node *ast);
static void translate_struct_decl(node *ast);
static void translate_member_decl(node *ast);
static void translate_struct_access(node *ast);
static void translate_label_decl(node *ast);
static void translate_goto_stmt(node *ast);
static void translate_array_init_expr(node *ast);
static void translate_array_access_expr(node *ast);
static void translate_while_stmt(node *ast);
static void translate_empty_expr(node *ast);
static void translate_neg_expr(node *ast);
static void translate_not_expr(node *ast);

static void print_ir(vector *ir);
static void do_translate(node *ast);

vector *translate(node *ast) {
    if (NULL != ast) {
        ir_list = mk_vector();

        if (NULL == ir_list) {
            log_error("Unable to allocate vector for IR list");
        }

        do_translate(ast);

        print_ir(ir_list);
    }

    return ir_list;
}

static void do_translate(node *ast) {
    if (NULL == ast) {
        log_error("Unable to access node for translation");
    }

    switch (ast->type) {
        case N_PROGRAM:
            translate_program(ast);
            break;
        case N_BLOCK_STMT:
            translate_block_stmt(ast);
            break;
        case N_VAR_DECL:
            translate_var_decl(ast);
            break;
        case N_FUNC_DECL:
            translate_func_decl(ast);
            break;
        case N_CALL_EXPR:
            translate_call_expr(ast);
            break;
        case N_FORMAL:
            translate_formal(ast);
            break;
        case N_IDENT:
            translate_ident(ast);
            break;
        case N_BINOP_EXPR:
            translate_binop_expr(ast);
            break;
        case N_ASSIGN_EXPR:
            translate_assign_expr(ast);
            break;
        case N_IF_STMT:
            translate_if_stmt(ast);
            break;
        case N_INTEGER_LITERAL:
        case N_FLOAT_LITERAL:
        case N_STRING_LITERAL:
        case N_BOOL_LITERAL:
            translate_literal(ast);
            break;
        case N_RETURN_STMT:
            translate_return_stmt(ast);
            break;
        case N_NIL:
            translate_nil(ast);
            break;
        case N_STRUCT_DECL:
            translate_struct_decl(ast);
            break;
        case N_MEMBER_DECL:
            translate_member_decl(ast);
            break;
        case N_STRUCT_ACCESS_EXPR:
            translate_struct_access(ast);
            break;
        case N_LABEL_DECL:
            translate_label_decl(ast);
            break;
        case N_GOTO_STMT:
            translate_goto_stmt(ast);
            break;
        case N_ARRAY_INIT_EXPR:
            translate_array_init_expr(ast);
            break;
        case N_ARRAY_ACCESS_EXPR:
            translate_array_access_expr(ast);
            break;
        case N_WHILE_STMT:
            translate_while_stmt(ast);
            break;
        case N_EMPTY_EXPR:
            translate_empty_expr(ast);
            break;
        case N_NEG_EXPR:
            translate_neg_expr(ast);
            break;
        case N_NOT_EXPR:
            translate_not_expr(ast);
            break;
        default:
            log_error("Unknown node type", ast);
            break;
    }
}

static void translate_program(node *ast) {
    assert(false && "Not implemented yet");
}

static void translate_block_stmt(node *ast) {
    assert(false && "Not implemented yet");
}

static void translate_var_decl(node *ast) {
    assert(false && "Not implemented yet");
}

static void translate_call_expr(node *ast) {
    assert(false && "Not implemented yet");
}

static void translate_func_decl(node *ast) {
    assert(false && "Not implemented yet");
}

static void translate_formal(node *ast) {
    assert(false && "Not implemented yet");
}

static void translate_ident(node *ast) {
    assert(false && "Not implemented yet");
}

static void translate_binop_expr(node *ast) {
    assert(false && "Not implemented yet");
}

static void translate_assign_expr(node *ast) {
    assert(false && "Not implemented yet");
}

static void translate_if_stmt(node *ast) {
    assert(false && "Not implemented yet");
}

static void translate_literal(node *ast) {
    assert(false && "Not implemented yet");
}

static void translate_return_stmt(node *ast) {
    assert(false && "Not implemented yet");
}

static void translate_nil(node *ast) {
    assert(false && "Not implemented yet");
}

static void translate_struct_decl(node *ast) {
    assert(false && "Not implemented yet");
}

static void translate_member_decl(node *ast) {
    assert(false && "Not implemented yet");
}

static void translate_struct_access(node *ast) {
    assert(false && "Not implemented yet");
}

static void translate_label_decl(node *ast) {
    assert(false && "Not implemented yet");
}

static void translate_goto_stmt(node *ast) {
    assert(false && "Not implemented yet");
}

static void translate_array_init_expr(node *ast) {
    assert(false && "Not implemented yet");
}

static void translate_array_access_expr(node *ast) {
    assert(false && "Not implemented yet");
}

static void translate_while_stmt(node *ast) {
    assert(false && "Not implemented yet");
}

static void translate_empty_expr(node *ast) {
    assert(false && "Not implemented yet");
}

static void translate_neg_expr(node *ast) {
    assert(false && "Not implemented yet");
}

static void translate_not_expr(node *ast) {
    assert(false && "Not implemented yet");
}

static void print_ir(vector *ir) {
    assert(false && "Not implemented yet");
}