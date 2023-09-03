/**
 * LBASIC Abstract Syntax Tree Module
 * file: ast.c
 * Author: Liam M. Murphy
 */

#include "ast.h"

#include "error.h"

#include <stdio.h>

data_type keyword_to_type(token_type t) {
    switch (t) {
        case T_INT:
            return D_INTEGER;
            break;
        case T_BOOL:
            return D_BOOLEAN;
            break;
        case T_STRING:
            return D_STRING;
            break;
        case T_FLOAT:
            return D_FLOAT;
            break;
        case T_VOID:
            return D_VOID;
            break;
        case T_STRUCT:
            return D_STRUCT;
            break;
        default:
            return D_UNKNOWN;
            break;
    }
}

char *type_to_str(data_type t) {
    switch (t) {
        case D_INTEGER:
            return "INTEGER";
            break;
        case D_FLOAT:
            return "FLOAT";
            break;
        case D_STRING:
            return "STRING";
            break;
        case D_BOOLEAN:
            return "BOOLEAN";
            break;
        case D_VOID:
            return "VOID";
            break;
        case D_NIL:
            return "NIL";
            break;
        case D_STRUCT:
            return "STRUCT";
            break;
        default:
            return "UNKNOWN";
            break;
    }
}

char *binop_to_str(token_type t) {
    switch (t) {
        case T_PLUS:
            return "+";
            break;
        case T_MINUS:
            return "-";
            break;
        case T_MUL:
            return "*";
            break;
        case T_DIV:
            return "/";
            break;
        case T_MOD:
            return "%";
            break;
        case T_AND:
            return "and";
            break;
        case T_OR:
            return "or";
            break;
        case T_EQ:
            return "==";
            break;
        case T_NE:
            return "!=";
            break;
        case T_GT:
            return ">";
            break;
        case T_GE:
            return ">=";
            break;
        case T_LT:
            return "<";
            break;
        case T_LE:
            return "<=";
            break;
        default:
            return "UNKNOWN";
            break;
    }
}

static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf(" ");
    }
}

void print_node(node *n, int indent) {
    if (n == NULL) {
        log_error("Unable to access node for printing");
    }

    print_indent(indent);
    switch (n->type) {
        case N_PROGRAM:
            printf("Program (\n");
            break;
        case N_BLOCK_STMT:
            printf("BlockStmt (\n");

            indent += INDENT_WIDTH;
            vecnode *bn = n->data.block_stmt.statements->head;
            while (bn != NULL) {
                node *n = bn->data;
                print_node(n, indent);

                bn = bn->next;
            }
            indent -= INDENT_WIDTH;

            print_indent(indent);
            printf("),\n");
            break;
        case N_VAR_DECL:
            printf("VarDecl (\n");
            print_indent(indent + INDENT_WIDTH);
            printf("Name: %s\n", n->data.var_decl.name);
            print_indent(indent + INDENT_WIDTH);
            printf("IsStruct: %s\n", (n->data.var_decl.is_struct ? "true" : "false"));
            print_indent(indent + INDENT_WIDTH);
            printf("IsArray: %s\n", (n->data.var_decl.is_array ? "true" : "false"));
            print_indent(indent + INDENT_WIDTH);
            printf("Dimensions: %d\n", n->data.var_decl.num_dimensions);
            print_indent(indent + INDENT_WIDTH);
            printf("Type: %s\n", type_to_str((data_type)n->data.var_decl.type));
            print_indent(indent + INDENT_WIDTH);
            printf("StructType: %s\n", n->data.var_decl.struct_type);
            print_indent(indent + INDENT_WIDTH);
            printf("Value: ");

            indent += INDENT_WIDTH;
            node *v = n->data.var_decl.value;
            if (v != NULL) {
                printf("\n");
                print_node(v, indent + INDENT_WIDTH);
            } else {
                printf("None\n");
            }
            indent -= INDENT_WIDTH;

            print_indent(indent);
            printf("),\n");
            break;
        case N_LABEL_DECL:
            printf("LabelDecl (\n");
            print_indent(indent + INDENT_WIDTH);
            printf("Name: %s\n", n->data.label_decl.name);
            print_indent(indent);
            printf("), \n");
            break;
        case N_GOTO_STMT:
            printf("GotoStmt (\n");
            print_indent(indent + INDENT_WIDTH);
            printf("Label: %s\n", n->data.goto_stmt.label);
            print_indent(indent);
            printf("), \n");
            break;
        case N_FUNC_DECL:
            printf("FuncDecl (\n");
            print_indent(indent + INDENT_WIDTH);
            printf("Name: %s\n", n->data.function_decl.name);
            print_indent(indent + INDENT_WIDTH);
            printf("Type: %s\n", type_to_str((data_type)n->data.function_decl.type));
            print_indent(indent + INDENT_WIDTH);
            printf("StructType: %s\n", n->data.function_decl.struct_type);
            print_indent(indent + INDENT_WIDTH);
            printf("IsStruct: %s\n", (n->data.function_decl.is_struct ? "true" : "false"));
            print_indent(indent + INDENT_WIDTH);
            printf("IsArray: %s\n", (n->data.function_decl.is_array ? "true" : "false"));
            print_indent(indent + INDENT_WIDTH);
            printf("Dimensions: %d\n", n->data.function_decl.num_dimensions);
            print_indent(indent + INDENT_WIDTH);
            printf("Formals ( ");
            indent += INDENT_WIDTH;
            if (n->data.function_decl.formals == NULL) {
                printf("None )\n");
            } else {
                printf("\n");
                vecnode *fn = n->data.function_decl.formals->head;

                while (fn != NULL) {
                    node *n = fn->data;
                    print_node(n, indent + INDENT_WIDTH);
                    fn = fn->next;
                }

                print_indent(indent);
                printf(")\n");
            }
            // Print FuncDecl body
            print_indent(indent);
            printf("Body:\n");

            print_node(n->data.function_decl.body, indent + INDENT_WIDTH);
            indent -= INDENT_WIDTH;

            print_indent(indent);
            printf("),\n");
            break;
        case N_RETURN_STMT:
            printf("ReturnStmt (\n");
            print_indent(indent + INDENT_WIDTH);
            printf("Expression: \n");
            indent += INDENT_WIDTH;
            print_node(n->data.return_stmt.expr, indent + INDENT_WIDTH);
            indent -= INDENT_WIDTH;
            print_indent(indent);
            printf("),\n");
            break;
        case N_CALL_EXPR:
            printf("CallExpr (\n");
            print_indent(indent + INDENT_WIDTH);
            printf("Function name: %s\n", n->data.call_expr.func_name);
            print_indent(indent + INDENT_WIDTH);
            printf("Args: ");
            if (n->data.call_expr.args == NULL) {
                printf("None\n");
            } else {
                indent += INDENT_WIDTH;
                printf("\n");
                vecnode *an = n->data.call_expr.args->head;

                while (an != NULL) {
                    node *n = an->data;
                    print_node(n, indent + INDENT_WIDTH);
                    an = an->next;
                }
                indent -= INDENT_WIDTH;
            }
            print_indent(indent);
            printf("),\n");
            break;
        case N_STRUCT_DECL:
            printf("StructDecl (\n");
            print_indent(indent + INDENT_WIDTH);
            printf("Name: %s\n", n->data.struct_decl.name);
            print_indent(indent + INDENT_WIDTH);
            printf("Members: ");

            indent += INDENT_WIDTH;
            vecnode *m = n->data.struct_decl.members->head;
            if (m != NULL) {
                printf("\n");
                while (m != NULL) {
                    print_node(m->data, indent + INDENT_WIDTH);
                    m = m->next;
                }
            } else {
                printf("None\n");
            }
            indent -= INDENT_WIDTH;

            print_indent(indent);
            printf("),\n");
            break;
        case N_STRUCT_ACCESS_EXPR:
            printf("StructAccessExpr (\n");
            print_indent(indent + INDENT_WIDTH);
            printf("Name: %s\n", n->data.struct_access.name);
            print_indent(indent + INDENT_WIDTH);
            printf("Member Name: %s\n", n->data.struct_access.member_name);
            print_indent(indent);
            printf("),\n");
            break;
        case N_ARRAY_INIT_EXPR:
            printf("ArrayInitExpr (\n");
            print_indent(indent + INDENT_WIDTH);
            printf("Expressions: \n");

            indent += INDENT_WIDTH;
            vecnode *e = n->data.array_init_expr.expressions->head;
            print_indent(indent + INDENT_WIDTH);
            printf("NumElements: %d\n", n->data.array_init_expr.expressions->count);

            if (e != NULL) {
                printf("\n");
                while (e != NULL) {
                    print_node(e->data, indent + INDENT_WIDTH);
                    e = e->next;
                }
            }
            indent -= INDENT_WIDTH;

            print_indent(indent);
            printf("), \n");
            break;
        case N_ARRAY_ACCESS_EXPR:
            printf("ArrayAccessExpr (\n");
            print_indent(indent + INDENT_WIDTH);
            printf("Expressions: \n");

            indent += INDENT_WIDTH;
            vecnode *el = n->data.array_access_expr.expressions->head;
            print_indent(indent + INDENT_WIDTH);
            printf("NumElements: %d\n", n->data.array_access_expr.expressions->count);

            if (el != NULL) {
                printf("\n");
                while (el != NULL) {
                    print_node(el->data, indent + INDENT_WIDTH);
                    el = el->next;
                }
            }
            indent -= INDENT_WIDTH;

            print_indent(indent);
            printf("), \n");
            break;
        case N_FORMAL:
            printf("Formal (\n");
            print_indent(indent + INDENT_WIDTH);
            printf("Name: %s\n", n->data.formal.name);
            print_indent(indent + INDENT_WIDTH);
            printf("IsStruct: %s\n", (n->data.formal.is_struct ? "true" : "false"));
            print_indent(indent + INDENT_WIDTH);
            printf("IsArray: %s\n", (n->data.formal.is_array ? "true" : "false"));
            print_indent(indent + INDENT_WIDTH);
            printf("Dimensions: %d\n", n->data.formal.num_dimensions);
            print_indent(indent + INDENT_WIDTH);
            printf("Type: %s\n", type_to_str((data_type)n->data.formal.type));
            print_indent(indent + INDENT_WIDTH);
            printf("StructType: %s\n", n->data.formal.struct_type);
            print_indent(indent);
            printf("),\n");
            break;
        case N_MEMBER_DECL:
            printf("MemberDecl (\n");
            print_indent(indent + INDENT_WIDTH);
            printf("Name: %s\n", n->data.member_decl.name);
            print_indent(indent + INDENT_WIDTH);
            printf("Type: %s\n", type_to_str((data_type)n->data.member_decl.type));
            print_indent(indent);
            printf("),\n");
            break;
        case N_INTEGER_LITERAL:
            printf("IntegerLiteral (\n");
            print_indent(indent + INDENT_WIDTH);
            printf("Type: %s\n", type_to_str((data_type)n->data.integer_literal.type));
            print_indent(indent + INDENT_WIDTH);
            printf("Value: %d\n", n->data.integer_literal.value);
            print_indent(indent);
            printf("),\n");
            break;
        case N_FLOAT_LITERAL:
            printf("FloatLiteral (\n");
            print_indent(indent + INDENT_WIDTH);
            printf("Type: %s\n", type_to_str((data_type)n->data.float_literal.type));
            print_indent(indent + INDENT_WIDTH);
            printf("Value: %f\n", n->data.float_literal.value);
            print_indent(indent);
            printf("),\n");
            break;
        case N_STRING_LITERAL:
            printf("StringLiteral (\n");
            print_indent(indent + INDENT_WIDTH);
            printf("Type: %s\n", type_to_str((data_type)n->data.string_literal.type));
            print_indent(indent + INDENT_WIDTH);
            printf("Value: %s\n", n->data.string_literal.value);
            print_indent(indent);
            printf("),\n");
            break;
        case N_BOOL_LITERAL:
            printf("BoolLiteral (\n");
            print_indent(indent + INDENT_WIDTH);
            printf("Type: %s\n", type_to_str((data_type)n->data.bool_literal.type));
            print_indent(indent + INDENT_WIDTH);
            printf("Value: %d\n", n->data.bool_literal.value);
            print_indent(indent + INDENT_WIDTH);
            printf("StringValue: %s\n", n->data.bool_literal.str_val);
            print_indent(indent);
            printf("),\n");
            break;
        case N_NIL:
            printf("Nil (\n");
            print_indent(indent + INDENT_WIDTH);
            printf("Value: %d\n", n->data.nil.value);
            print_indent(indent);
            printf("),\n");
            break;
        case N_IDENT:
            printf("Identifier (\n");
            print_indent(indent + INDENT_WIDTH);
            printf("Name: %s\n", n->data.identifier.name);
            print_indent(indent);
            printf("),\n");
            break;
        case N_IF_STMT:
            printf("IfStmt (\n");
            print_indent(indent + INDENT_WIDTH);
            printf("Test: \n");

            indent += INDENT_WIDTH;
            print_node(n->data.if_stmt.test, indent + INDENT_WIDTH);
            indent -= INDENT_WIDTH;

            print_indent(indent + INDENT_WIDTH);
            printf("Body: \n");

            indent += INDENT_WIDTH;
            print_node(n->data.if_stmt.body, indent + INDENT_WIDTH);
            indent -= INDENT_WIDTH;

            print_indent(indent + INDENT_WIDTH);
            printf("Else: ");

            if (n->data.if_stmt.else_stmt == NULL) {
                printf("None \n");
            } else {
                printf("\n");
                indent += INDENT_WIDTH;
                print_node(n->data.if_stmt.else_stmt, indent + INDENT_WIDTH);
                indent -= INDENT_WIDTH;
            }

            print_indent(indent);
            printf("),\n");
            break;
        case N_WHILE_STMT:
            printf("WhileStmt (\n");
            print_indent(indent + INDENT_WIDTH);
            printf("Test: \n");

            indent += INDENT_WIDTH;
            print_node(n->data.while_stmt.test, indent + INDENT_WIDTH);
            indent -= INDENT_WIDTH;

            print_indent(indent + INDENT_WIDTH);
            printf("Body: \n");

            indent += INDENT_WIDTH;
            print_node(n->data.while_stmt.body, indent + INDENT_WIDTH);
            indent -= INDENT_WIDTH;

            print_indent(indent);
            printf("),\n");
            break;
        case N_EMPTY_EXPR:
            printf("EmptyExpr (),\n");
            break;
        case N_NEG_EXPR:
            printf("NegExpr (\n");
            print_indent(indent + INDENT_WIDTH);
            printf("Expr: \n");

            indent += INDENT_WIDTH;
            print_node(n->data.neg_expr.expr, indent + INDENT_WIDTH);
            indent -= INDENT_WIDTH;

            print_indent(indent);
            printf(")\n");
            break;
        case N_NOT_EXPR:
            printf("NotExpr (\n");
            print_indent(indent + INDENT_WIDTH);
            printf("Expr: \n");

            indent += INDENT_WIDTH;
            print_node(n->data.not_expr.expr, indent + INDENT_WIDTH);
            indent -= INDENT_WIDTH;

            print_indent(indent);
            printf(")\n");
            break;
        case N_BINOP_EXPR:
            printf("BinOpExpr (\n");
            print_indent(indent + INDENT_WIDTH);
            printf("LHS: \n");

            indent += INDENT_WIDTH;
            print_node(n->data.bin_op_expr.lhs, indent + INDENT_WIDTH);
            indent -= INDENT_WIDTH;

            print_indent(indent + INDENT_WIDTH);
            printf("Operator: %s\n", binop_to_str(n->data.bin_op_expr.operator));

            print_indent(indent + INDENT_WIDTH);
            printf("RHS: \n");
            indent += INDENT_WIDTH;
            print_node(n->data.bin_op_expr.rhs, indent + INDENT_WIDTH);
            indent -= INDENT_WIDTH;

            print_indent(indent);
            printf("),\n");
            break;
        case N_ASSIGN_EXPR:
            printf("AssignExpr (\n");
            print_indent(indent + INDENT_WIDTH);
            printf("LHS: \n");

            indent += INDENT_WIDTH;
            print_node(n->data.assign_expr.lhs, indent + INDENT_WIDTH);
            indent -= INDENT_WIDTH;

            print_indent(indent + INDENT_WIDTH);
            printf("RHS: \n");
            indent += INDENT_WIDTH;
            print_node(n->data.assign_expr.rhs, indent + INDENT_WIDTH);
            indent -= INDENT_WIDTH;

            print_indent(indent);
            printf("),\n");
            break;
        default:
            printf("Unknown node type: %d\n", n->type);
    }
}

void print_ast(node *ast) {
    int indent = 0;

    if (ast != NULL) {
        // Print top level
        print_node(ast, indent);
        indent += INDENT_WIDTH;

        // Print children
        print_indent(indent);
        printf("Statements (");
        if (ast->data.program.statements->head != NULL) {
            printf("\n");
            indent += INDENT_WIDTH;
            vecnode *vn = ast->data.program.statements->head;
            while (vn != NULL) {
                node *n = vn->data;
                print_node(n, indent);

                vn = vn->next;
            }

            indent -= INDENT_WIDTH;
            print_indent(indent);
        } else {
            printf("None");
        }
        printf(")\n");
    }

    indent -= INDENT_WIDTH;
    printf(")\n");
    return;
}
