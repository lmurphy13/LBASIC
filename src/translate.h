#if defined(HAS_TRANSLATOR)

#ifndef TRANSLATE_H
#define TRANSLATE_H

#include "ast.h"
#include "symtab.h"
#include "vector.h"

// 64 bytes + 1 null byte
#define MAX_ARGUMENT 65
#define MAX_COMMENT 65

typedef enum ir_type {
    IR_PUSH,  // Push to the stack
    IR_POP,   // Pop from the stack
    IR_LOAD,  // Load a value into a temporary
    IR_STORE, // Store a value in memory
    IR_ADD,
    IR_SUB,
    IR_MUL,
    IR_DIV,
    IR_CALL,   // Call a function
    IR_JUMP,   // Jump to a label
    IR_CMP,    // Compare two values
    IR_LABEL,  // Create a label
    IR_RETURN, // Return from a function
    NUM_IR_TYPES
} ir_type;

// Three address code
typedef struct ir_node {
    ir_type type;
    unsigned int num_args;
    char arg1[MAX_ARGUMENT];
    char arg2[MAX_ARGUMENT];
    char arg3[MAX_ARGUMENT];
    binding_t *arg1_binding;
    binding_t *arg2_binding;
    binding_t *arg3_binding;
    char comment[MAX_COMMENT];
} ir_node;

// Prototypes
vector *translate(node *ast);

#endif // TRANSLATE_H

#endif // HAS_TRANSLATOR