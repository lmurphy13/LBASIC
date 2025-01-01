/**
 * LBASIC Symbol Table Public Definitions
 * File: symtab.h
 * Author: Liam M. Murphy
 */

#ifndef SYMTAB_H
#define SYMTAB_H

#include "ast.h"
#include "hashtable.h"
#include "token.h"
#include "vector.h"

#include <stdlib.h>

typedef enum symbol_type_s {
    SYMBOL_TYPE_FUNCTION  = 0,
    SYMBOL_TYPE_VARIABLE  = 1,
    SYMBOL_TYPE_FORMAL    = 2,
    SYMBOL_TYPE_STRUCTURE = 3,
    SYMBOL_TYPE_MEMBER    = 4,
    SYMBOL_TYPE_UNKNOWN   = 5,
    NUM_SYMBOL_TYPES      = 6
} symbol_type_t;

typedef struct b_function_s {
    data_type return_type;
    char struct_type[MAX_LITERAL];
    bool is_array_type;
    bool is_struct_type;
    unsigned int num_dimensions;
    unsigned int num_args;
    vector *formals; // vector of formal_t, one for each formal argument
} b_function_t;

// Can be used for either variables or formal args.
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
    char name[MAX_LITERAL];
    //    bool seen;
    hashtable *table;
    struct symtab_s *prev;
    struct symtab_s *next;
} symtab_t;

/* Symbol table interface */
symtab_t *symtab_new(void);
void symtab_insert(symtab_t *st, binding_t *binding);
binding_t *symtab_lookup(symtab_t *st, char *identifier, bool single_scope);
void symtab_free(symtab_t *st);

binding_t *mk_binding(symbol_type_t);

// Hashtable comparison callback. Tries to find match using binding_t*
bool ht_compare_binding(vecnode *vn, void *key);

void print_binding(const binding_t *);
void print_symbol_table(const symtab_t *);
// sym_data_type ast_data_type_to_binding_data_type(data_type t);
// data_type sym_data_to_data_type(sym_data_type t);

#endif // SYMTAB_H
