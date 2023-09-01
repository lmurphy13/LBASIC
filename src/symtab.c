/**
 * LBASIC Symbol Table Module
 * File: symtab.h
 * Author: Liam M. Murphy
 */

#include "symtab.h"

#include "error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

binding_t *mk_binding() {
    binding_t *retval = (binding_t *)malloc(sizeof(binding_t));

    if (retval != NULL) {
        memset(retval, 0, sizeof(binding_t));

        snprintf(retval->struct_type, sizeof(retval->struct_type), "NONE");
        snprintf(retval->name, sizeof(retval->name), "NONE");
    } else {
        log_error("Unable to allocate binding_t");
    }

    return retval;
}

bool ht_compare_binding(vecnode *vn, void *key) {
    bool retval = false;

    binding_t *b = (binding_t *)vn->data;
    retval       = (strcmp(b->name, (char *)key) == 0);

    return retval;
}

static char *b_data_type_to_str(sym_data_type t) {
    switch (t) {
    case SYM_DTYPE_INTEGER:
        return "SYM_DTYPE_INTEGER";
        break;
    case SYM_DTYPE_FLOAT:
        return "SYM_DTYPE_FLOAT";
        break;
    case SYM_DTYPE_STRING:
        return "SYM_DTYPE_STRING";
        break;
    case SYM_DTYPE_BOOLEAN:
        return "SYM_DTYPE_BOOLEAN";
        break;
    case SYM_DTYPE_VOID:
        return "SYM_DTYPE_VOID";
        break;
    case SYM_DTYPE_STRUCT:
        return "SYM_DTYPE_STRUCT";
        break;
    case SYM_DTYPE_UNKNOWN:
        return "SYM_DTYPE_UNKNOWN";
    default:
        return "UNKNOWN";
        break;
    }
}

static char *b_obj_type_to_str(sym_obj_type t) {
    switch (t) {
    case SYM_OTYPE_FUNCTION:
        return "SYM_OTYPE_FUNCTION";
        break;
    case SYM_OTYPE_VARIABLE:
        return "SYM_OTYPE_VARIABLE";
        break;
    case SYM_OTYPE_STRUCTURE:
        return "SYM_OTYPE_STRUCTURE";
        break;
    case SYM_OTYPE_UNKNOWN:
        return "SYM_OTYPE_STRUCTURE";
        break;
    default:
        return "UNKNOWN";
        break;
    }
}

static data_type sym_data_to_data_type(sym_data_type t) {
    switch (t) {
    case SYM_DTYPE_INTEGER:
        return D_INTEGER;
        break;
    case SYM_DTYPE_FLOAT:
        return D_FLOAT;
        break;
    case SYM_DTYPE_STRING:
        return D_STRING;
        break;
    case SYM_DTYPE_BOOLEAN:
        return D_BOOLEAN;
        break;
    case SYM_DTYPE_VOID:
        return D_VOID;
        break;
    case SYM_DTYPE_STRUCT:
        return D_STRUCT;
        break;
    case SYM_DTYPE_UNKNOWN:
    default:
        return D_UNKNOWN;
        break;
    }
}

sym_data_type ast_data_type_to_binding_data_type(data_type t) {
    switch (t) {
    case D_INTEGER:
        return SYM_DTYPE_INTEGER;
    case D_FLOAT:
        return SYM_DTYPE_FLOAT;
    case D_STRING:
        return SYM_DTYPE_STRING;
    case D_BOOLEAN:
        return SYM_DTYPE_BOOLEAN;
    case D_VOID:
        return SYM_DTYPE_VOID;
    case D_STRUCT:
        return SYM_DTYPE_STRUCT;
    default:
        return SYM_DTYPE_UNKNOWN;
    }
}

void print_binding(const binding_t *b) {
    if (b != NULL) {
        printf("Binding Name: %s | ", b->name);
        printf("Binding Struct Type: %s | ", b->struct_type);
        printf("Binding Data Type: %s | ", b_data_type_to_str(b->data_type));
        printf("Binding Object Type: %s | ", b_obj_type_to_str(b->object_type));
        printf("Binding IsArray: %d | ", b->is_array);
        printf("Binding Array Dimensions: %u\n", b->array_dims);
    }
}

symtab_t *symtab_new() {
    symtab_t *new = (symtab_t *)malloc(sizeof(symtab_t) + 1);
    memset(new, 0, sizeof(symtab_t));
    new->next = NULL;
    new->prev = NULL;

    hashtable *ht = mk_hashtable();
    new->table    = ht;

    return new;
}

void symtab_free(symtab_t *st) {
    // Seek to end
    while (st->next != NULL) {
        st = st->next;
    }

    // Walk back up the list
    while (st->prev != NULL) {
        st = st->prev;

        ht_free(st->next->table);
        free(st->next);
    }

    if (st != NULL) {
        free(st);
    }
}

void symtab_append(symtab_t *st, symtab_t *new_st) {
    if (st != NULL) {
        if (new_st != NULL) {
            while (st->next != NULL) {
                st = symtab_next(st);
            }

            if (st->next == NULL) {
                new_st->prev = st;
                new_st->next = NULL;

                st->next = new_st;
            }
        } else {
            log_error("Cannot access new_st");
        }
    } else {
        log_error("Cannot access st");
    }
}

symtab_t *symtab_next(symtab_t *st) {
    if (st == NULL) {
        return NULL;
    } else {
        return st->next;
    }
}

symtab_t *symtab_prev(symtab_t *st) {
    if (st == NULL) {
        printf("null element\n");
        return NULL;
    } else {
        return st->prev;
    }
}
