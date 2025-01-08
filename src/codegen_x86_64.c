#if defined(HAS_CODEGEN)

#include "codegen_x86_64.h"

#include "assert.h"
#include "error.h"
#include "vector.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * https://www.cs.uaf.edu/2017/fall/cs301/reference/x86_64.html
 * https://cs.brown.edu/courses/cs033/docs/guides/x64_cheatsheet.pdf
 */
typedef enum reg_e {
    RAX = 0, // Accumulator                                  Return value
    RBX,     // Base
    RCX,     // Counter                                      Arg 4
    RDX,     // Data                                         Arg 3
    RSI,     // Source index for string operations           Arg 2
    RDI,     // Destination index for string operations      Arg 1
    RSP,     // Stack pointer                                Top of stack
    RBP,     // Base pointer                                 Base of stack
    R8,      // General purpose                              Arg 5
    R9,      // General purpose                              Arg 6
    R10,     // General purpose
    R11,     // General purpose
    R12,     // General purpose
    R13,     // General purpose
    R14,     // General purpose
    R15,     // General purpose
    N_REGS
} reg_t;

const unsigned int NUM_ARG_REGS = 6;
reg_t arg_regs[]                = {RDI, RSI, RDX, RCX, R8, R9};

// Volatile
const unsigned int NUM_CALLER_SAVES = 10;
reg_t caller_save[]                 = {RAX, RCX, RDX, RDI, RSI, RSP, R8, R9, R10, R11};

// Non-volatile
const unsigned int NUM_CALLEE_SAVES = 6;
reg_t callee_save[]                 = {RBX, RBP, R12, R13, R14, R15};

static FILE *assem_fp = NULL;

static void do_codegen(node *ast);
static void codegen_program(node *ast);
static void codegen_block_stmt(node *ast);
static void codegen_var_decl(node *ast);
static void codegen_func_decl(node *ast);
static void codegen_call_expr(node *ast);
static void codegen_formal(node *ast);
static void codegen_ident(node *ast);
static void codegen_binop_expr(node *ast);
static void codegen_assign_expr(node *ast);
static void codegen_if_stmt(node *ast);
static void codegen_literal(node *ast);
static void codegen_return_stmt(node *ast);
static void codegen_nil(node *ast);
static void codegen_struct_decl(node *ast);
static void codegen_member_decl(node *ast);
static void codegen_struct_access(node *ast);
static void codegen_label_decl(node *ast);
static void codegen_goto_stmt(node *ast);
static void codegen_array_init_expr(node *ast);
static void codegen_array_access_expr(node *ast);
static void codegen_while_stmt(node *ast);
static void codegen_empty_expr(node *ast);
static void codegen_neg_expr(node *ast);
static void codegen_not_expr(node *ast);
static bool is_int_literal(node *ast);
static bool is_float_literal(node *ast);
static bool is_string_literal(node *ast);
static bool is_bool_literal(node *ast);

static const char *const reg(reg_t r) {
    switch (r) {
        case RAX:
            return "\%rax";
        case RBX:
            return "\%rbx";
        case RCX:
            return "\%rcx";
        case RDX:
            return "\%rdx";
        case RSI:
            return "\%rsi";
        case RDI:
            return "\%rdi";
        case RSP:
            return "\%rsp";
        case RBP:
            return "\%rbp";
        case R8:
            return "\%r8";
        case R9:
            return "\%r9";
        case R10:
            return "\%r10";
        case R11:
            return "\%r11";
        case R12:
            return "\%r12";
        case R13:
            return "\%r13";
        case R14:
            return "\%r14";
        case R15:
            return "\%r15";
        default:
            log_error("Unknown register %d", r);
    }
    return "ERR";
}

static void emit(const char *format, ...) {
    va_list args;
    va_start(args, format);

    vfprintf(assem_fp, format, args);

    va_end(args);
    fprintf(assem_fp, "\n");
}

static bool is_int_literal(node *ast) { return (ast->type == N_INTEGER_LITERAL); }

static bool is_float_literal(node *ast) { return (ast->type == N_FLOAT_LITERAL); }

static bool is_bool_literal(node *ast) { return (ast->type == N_BOOL_LITERAL); }

static bool is_string_literal(node *ast) { return (ast->type == N_STRING_LITERAL); }

void codegen(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for code generation", __FUNCTION__);
    }

    // Create new file for assembly code
    assem_fp = fopen("program.s", "w+");

    if (NULL != assem_fp) {
        do_codegen(ast);
        fclose(assem_fp);
    } else {
        log_error("Unable to create assembly file.");
    }
}

static void do_codegen(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for code generation", __FUNCTION__);
    }

    switch (ast->type) {
        case N_PROGRAM:
            codegen_program(ast);
            break;
        case N_BLOCK_STMT:
            codegen_block_stmt(ast);
            break;
        case N_VAR_DECL:
            codegen_var_decl(ast);
            break;
        case N_FUNC_DECL:
            codegen_func_decl(ast);
            break;
        case N_CALL_EXPR:
            codegen_call_expr(ast);
            break;
        case N_FORMAL:
            codegen_formal(ast);
            break;
        case N_IDENT:
            codegen_ident(ast);
            break;
        case N_BINOP_EXPR:
            codegen_binop_expr(ast);
            break;
        case N_ASSIGN_EXPR:
            codegen_assign_expr(ast);
            break;
        case N_IF_STMT:
            codegen_if_stmt(ast);
            break;
        case N_INTEGER_LITERAL:
        case N_FLOAT_LITERAL:
        case N_STRING_LITERAL:
        case N_BOOL_LITERAL:
            codegen_literal(ast);
            break;
        case N_RETURN_STMT:
            codegen_return_stmt(ast);
            break;
        case N_NIL:
            codegen_nil(ast);
            break;
        case N_STRUCT_DECL:
            codegen_struct_decl(ast);
            break;
        case N_MEMBER_DECL:
            codegen_member_decl(ast);
            break;
        case N_STRUCT_ACCESS_EXPR:
            codegen_struct_access(ast);
            break;
        case N_LABEL_DECL:
            codegen_label_decl(ast);
            break;
        case N_GOTO_STMT:
            codegen_goto_stmt(ast);
            break;
        case N_ARRAY_INIT_EXPR:
            codegen_array_init_expr(ast);
            break;
        case N_ARRAY_ACCESS_EXPR:
            codegen_array_access_expr(ast);
            break;
        case N_WHILE_STMT:
            codegen_while_stmt(ast);
            break;
        case N_EMPTY_EXPR:
            codegen_empty_expr(ast);
            break;
        case N_NEG_EXPR:
            codegen_neg_expr(ast);
            break;
        case N_NOT_EXPR:
            codegen_not_expr(ast);
            break;
        default:
            log_error("Unknown node type", ast);
            break;
    }
}

static void codegen_program(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for code generation", __FUNCTION__);
    }

    emit("#include \"lbasic_print.h\"\n");
    emit("\t.file\t\"program.s\"");
    emit("\t.text");
    emit("\t.globl\tmain\n");

    emit("main:");
    emit("\tpushq\t%s\t\t# Save base pointer", reg(RBP));
    emit("\tmovq\t%s,\t%s\t\t# Set the stack pointer to the bottom of the stack", reg(RSP),
         reg(RBP));

    // Generate code for children
    if (ast->data.program.statements->head != NULL) {
        vecnode *vn = ast->data.program.statements->head;
        while (vn != NULL) {
            node *n = vn->data;

            if (NULL != n) {
                do_codegen(n);
                vn = vn->next;
            }
        }
    }

    // emit("\tpop\t%s\t\t# Reset base pointer to main caller address", reg(RBP));
    emit("\tleave");
    emit("\tret\t\t# Exit the program");
}

static void codegen_block_stmt(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for code generation", __FUNCTION__);
    }
    assert(false && "Not yet implemented");
}

static void codegen_var_decl(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for code generation", __FUNCTION__);
    }

    if (NULL != ast->data.var_decl.value) {
        char *identifier_name = ast->data.var_decl.name;
        node *rhs             = ast->data.var_decl.value;

        if (is_int_literal(rhs)) {
            emit("\tpushq\t$%d\t\t# %s", rhs->data.integer_literal.value, identifier_name);
        } else {
            do_codegen(rhs);

            // Pop result into rdx
            emit("\tpop\t%s", reg(RDX));
            // log_error("RHS type %d not supported yet", rhs->type);
        }
    }
}

static void codegen_func_decl(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for code generation", __FUNCTION__);
    }
    assert(false && "Not yet implemented");
}

static void codegen_call_expr(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for code generation", __FUNCTION__);
    }

    // Save all caller-save registers
    // for (int i = 0; i < NUM_CALLER_SAVES; i++) {
    //     emit("\tpushq\t%s", reg(caller_save[i]));
    // }

    if (NULL != ast->data.call_expr.args) {
        const int num_args = vector_length(ast->data.call_expr.args);
        if (num_args <= NUM_ARG_REGS) {
            unsigned int r = 0;
            int offset     = -4;
            for (r = 0; r < num_args; r++) {
                emit("\tmovq\t%s,\t%d(%s)\t\t# Arg %d", reg(arg_regs[r]), offset, reg(RBP), r + 1);
                offset -= 4;
            }
        }
        emit("\tcall\t%s", ast->data.call_expr.func_name);
    }

    // Restore caller-save registers
    // for (int i = NUM_CALLER_SAVES - 1; i >= 0; i--) {
    //     //emit("\tpopq\t%s", reg(caller_save[i]));
    //     emit("\tpopq\t%s", reg(caller_save[i]));
    // }

    // // Prologue
    // emit("\tpushq\t%s", reg(RBX));
    // emit("\tmovq\t%s,\t%s", reg(RSP), reg(RBP));

    // // Create space for locals (8 byte aligned)
    // emit("\tsubq\t$0x10,\t%s", reg(RSP));

    // emit("\tcall\t%s", ast->data.call_expr.func_name);

    // // Epilogue
    // emit("\taddq\t$0x10,\t%s# Destroy stack space", reg(RSP));
    // emit("\tpopq\t%s\t\t# Destroy stack frame", reg(RBP));
    // emit("\tret\t\t# Return to caller");
}

static void codegen_formal(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for code generation", __FUNCTION__);
    }
    assert(false && "Not yet implemented");
}

static void codegen_ident(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for code generation", __FUNCTION__);
    }
    assert(false && "Not yet implemented");
}

static void codegen_binop_expr(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for code generation", __FUNCTION__);
    }

    node *lhs = ast->data.bin_op_expr.lhs;
    node *rhs = ast->data.bin_op_expr.rhs;

    do_codegen(rhs);
    do_codegen(lhs);

    switch (ast->data.bin_op_expr.operator) {
        case T_PLUS:
            emit("\tpopq\t%s\t\t# %s", reg(RAX), "RHS");
            emit("\tpopq\t%s\t\t# %s", reg(RCX), "LHS");
            //             SRC     DST
            emit("\tadd\t%s,\t%s", reg(RCX), reg(RAX));
            // Push result back onto the stack
            emit("\tpushq\t%s", reg(RAX));
            break;
        case T_MUL: // SOURCE is RCX. DEST is RAX (implicit)
            emit("\tpopq\t%s\t\t# %s", reg(RAX), "RHS");
            emit("\tpopq\t%s\t\t# %s", reg(RCX), "LHS");
            emit("\tmul\t%s", reg(RCX));
            // Push result back onto the stack
            emit("\tpushq\t%s", reg(RAX));
            break;
        default:
            log_error("Operator type %d not implemented yet", ast->data.bin_op_expr.operator);
    }
}

static void codegen_assign_expr(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for code generation", __FUNCTION__);
    }
    assert(false && "Not yet implemented");
}

static void codegen_if_stmt(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for code generation", __FUNCTION__);
    }
    assert(false && "Not yet implemented");
}

static void codegen_literal(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for code generation", __FUNCTION__);
    }

    switch (ast->type) {
        case N_INTEGER_LITERAL:
            emit("\tpushq\t$%d", ast->data.integer_literal.value);
            break;
        case N_FLOAT_LITERAL:
        case N_BOOL_LITERAL:
        case N_STRING_LITERAL:
        default:
            log_error("Literal type %d not yet implemented", ast->type);
    }
}

static void codegen_return_stmt(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for code generation", __FUNCTION__);
    }
    assert(false && "Not yet implemented");
}

static void codegen_nil(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for code generation", __FUNCTION__);
    }
    assert(false && "Not yet implemented");
}

static void codegen_struct_decl(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for code generation", __FUNCTION__);
    }
    assert(false && "Not yet implemented");
}

static void codegen_member_decl(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for code generation", __FUNCTION__);
    }
    assert(false && "Not yet implemented");
}

static void codegen_struct_access(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for code generation", __FUNCTION__);
    }
    assert(false && "Not yet implemented");
}

static void codegen_while_stmt(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for code generation", __FUNCTION__);
    }
    assert(false && "Not yet implemented");
}

static void codegen_empty_expr(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for code generation", __FUNCTION__);
    }
    assert(false && "Not yet implemented");
}

static void codegen_neg_expr(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for code generation", __FUNCTION__);
    }
    assert(false && "Not yet implemented");
}

static void codegen_not_expr(node *ast) {
    if (NULL == ast) {
        log_error("%s(): Unable to access node for code generation", __FUNCTION__);
    }
    assert(false && "Not yet implemented");
}

/* Language functionality generated below is not yet supported */
static void codegen_label_decl(node *ast) { assert(false && "Not yet implemented"); }

static void codegen_goto_stmt(node *ast) { assert(false && "Not yet implemented"); }

static void codegen_array_init_expr(node *ast) { assert(false && "Not yet implemented"); }

static void codegen_array_access_expr(node *ast) { assert(false && "Not yet implemented"); }

#endif // HAS_CODEGEN