#ifndef TRANSLATE_H
#define TRANSLATE_H

#include "ast.h"
#include "vector.h"

// 64 bytes + 1 null byte
#define MAX_OPERAND 65

typedef enum ir_type {
    IR_PUSH,
    IR_POP,
    IR_LOAD,
    IR_STORE,
    IR_ADD,
    IR_SUB,
    IR_MUL,
    IR_DIV,
    IR_CALL,
    IR_JUMP,
    IR_CMP,
    IR_MOVE,
    IR_LABEL,
    IR_RETURN,
    NUM_IR_TYPES
} ir_type;

typedef struct ir_push_s {

} ir_push_t;

typedef struct ir_pop_s {

} ir_pop_t;

typedef struct ir_load_s {

} ir_load_t;

typedef struct ir_store_s {

} ir_store_t;

typedef struct ir_add_s {

} ir_add_t;

typedef struct ir_sub_s {

} ir_sub_t;

typedef struct ir_mul_s {

} ir_mul_t;

typedef struct ir_div_s {

} ir_div_t;

typedef struct ir_call_s {

} ir_call_t;

typedef struct ir_jump_s {

} ir_jump_t;

typedef struct ir_cmp_s {

} ir_cmp_t;

typedef struct ir_move_s {

} ir_move_t;

typedef struct ir_label_s {

} ir_label_t;

typedef struct ir_return_s {

} ir_return_t;

typedef struct ir_node {
    ir_type type;
    char operand[MAX_OPERAND];
    union {
        ir_push_t push;
        ir_pop_t pop;
        ir_load_t load;
        ir_store_t store;
        ir_add_t add;
        ir_sub_t sub;
        ir_mul_t mul;
        ir_div_t div;
        ir_call_t call;
        ir_jump_t jump;
        ir_cmp_t cmp;
        ir_move_t move;
        ir_label_t label;
        ir_return_t ret;
    } data;
} ir_node;

// Prototypes
vector *translate(node *ast);

#endif // TRANSLATE_H