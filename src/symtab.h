/**
 * LBASIC Symbol Table Public Definitions
 * File: symtab.h
 * Author: Liam M. Murphy
 */

#ifndef SYMTAB_H
#define SYMTAB_H

#include "ast.h"
#include "token.h"
#include "utils.h"

#include <stdlib.h>

typedef enum symbol_type_s {
    SYMBOL_TYPE_FUNCTION  = 0,
    SYMBOL_TYPE_VARIABLE  = 1,
    SYMBOL_TYPE_STRUCTURE = 2,
    SYMBOL_TYPE_MEMBER    = 3,
    SYMBOL_TYPE_UNKNOWN   = 4,
    NUM_SYMBOL_TYPES      = 5
} symbol_type_t;

typedef struct b_function_s {
    data_type return_type;
    char struct_type[MAX_LITERAL];
    bool is_array_type;
    bool is_struct_type;
    unsigned int num_dimensions;
    unsigned int num_args;
} b_function_t;

typedef struct b_variable_s {
    data_type type;
    char struct_type[MAX_LITERAL];
    bool is_array_type;
    bool is_struct_type;
    unsigned int num_dimensions;
} b_variable_t;

typedef struct b_structure_s {
    char struct_type[MAX_LITERAL];
    unsigned int num_members;
} b_structure_t;

typedef struct b_member_s {
    data_type type;
    char struct_type[MAX_LITERAL];
    bool is_array_type;
    bool is_struct_type;
    unsigned int num_dimensions;
} b_member_t;

typedef struct binding_s {
    char name[MAX_LITERAL];
    symbol_type_t symbol_type;
    union {
        b_function_t function_type;
        b_variable_t variable_type;
        b_structure_t structure_type;
        b_member_t member_type;
    } data;
} binding_t;

typedef struct symtab_s {
    unsigned int level;
    hashtable *table;
} symtab_t;

symtab_t *symtab_new(void);
void symtab_free(symtab_t *st);

binding_t *mk_binding(symbol_type_t);

// Hashtable comparison callback. Tries to find match using binding_t*
bool ht_compare_binding(vecnode *vn, void *key);

void print_binding(const binding_t *);
void print_table(const symtab_t *);
// sym_data_type ast_data_type_to_binding_data_type(data_type t);
// data_type sym_data_to_data_type(sym_data_type t);

#endif // SYMTAB_H
