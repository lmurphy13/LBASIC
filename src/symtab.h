/**
 * LBASIC Symbol Table Public Definitions
 * File: symtab.h
 * Author: Liam M. Murphy
 */

#ifndef SYMTAB_H
#define SYMTAB_H

#include "token.h"
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
} binding_t;

binding_t *mk_binding(void);

#endif // SYMTAB_H
