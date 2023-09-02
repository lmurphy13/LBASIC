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

typedef enum sym_data_type {
    SYM_DTYPE_INTEGER = 0,
    SYM_DTYPE_FLOAT   = 1,
    SYM_DTYPE_STRING  = 2,
    SYM_DTYPE_BOOLEAN = 3,
    SYM_DTYPE_VOID    = 4,
    SYM_DTYPE_STRUCT  = 5,
    SYM_DTYPE_UNKNOWN = 6,
    NUM_SYM_DTYPES    = 7
} sym_data_type;

typedef enum sym_obj_type {
    SYM_OTYPE_FUNCTION  = 0,
    SYM_OTYPE_VARIABLE  = 1,
    SYM_OTYPE_STRUCTURE = 2,
    SYM_OTYPE_UNKNOWN   = 3,
    NUM_SYM_OTYPES      = 4
} sym_obj_type;

typedef struct binding_s {
    char name[MAX_LITERAL];
    char struct_type[MAX_LITERAL];
    sym_data_type data_type;
    sym_obj_type object_type;
    bool is_array;
    unsigned int array_dims;
} binding_t;

typedef struct symtab_s {
    unsigned int level;
    hashtable *table;
    struct symtab_s *prev;
    struct symtab_s *next;
} symtab_t;

symtab_t *symtab_new(void);
void symtab_free(symtab_t *st);
void symtab_append(symtab_t *st, symtab_t *new_st);
symtab_t *symtab_next(symtab_t *st);
symtab_t *symtab_prev(symtab_t *st);

binding_t *mk_binding(void);

// Hashtable comparison callback. Tries to find match using binding_t*
bool ht_compare_binding(vecnode *vn, void *key);

void print_binding(const binding_t *);
sym_data_type ast_data_type_to_binding_data_type(data_type t);
data_type sym_data_to_data_type(sym_data_type t);

#endif // SYMTAB_H
