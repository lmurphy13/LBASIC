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

binding_t *mk_binding(symbol_type_t st) {
    binding_t *retval = (binding_t *)malloc(sizeof(binding_t));

    if (retval != NULL) {
        memset(retval, 0, sizeof(binding_t));
        retval->symbol_type = st;
        snprintf(retval->name, sizeof(retval->name), "NONE");

        switch (st) {
            case SYMBOL_TYPE_FUNCTION:
                snprintf(retval->data.function_type.struct_type,
                         sizeof(retval->data.function_type.struct_type), "NONE");
                break;
            case SYMBOL_TYPE_VARIABLE:
                snprintf(retval->data.variable_type.struct_type,
                         sizeof(retval->data.variable_type.struct_type), "NONE");
                break;
            case SYMBOL_TYPE_STRUCTURE:
                snprintf(retval->data.structure_type.struct_type,
                         sizeof(retval->data.structure_type.struct_type), "NONE");
                break;
            case SYMBOL_TYPE_MEMBER:
                snprintf(retval->data.member_type.struct_type,
                         sizeof(retval->data.member_type.struct_type), "NONE");
                break;
            case SYMBOL_TYPE_UNKNOWN:
            default: {
                char msg[MAX_ERROR_LEN] = {'\0'};
                snprintf(msg, sizeof(msg), "Unknown symbol type %d", st);
                log_error(msg);
            }
        }
    } else {
        log_error("Unable to allocate binding_t");
    }

    return retval;
}

// Compare bindings by name
bool ht_compare_binding(vecnode *vn, void *key) {
    bool retval = false;

    binding_t *b = (binding_t *)vn->data;
    retval       = (strcmp(b->name, (char *)key) == 0);

    return retval;
}

static char *sym_type_to_str(symbol_type_t t) {
    switch (t) {
        case SYMBOL_TYPE_FUNCTION:
            return "SYMBOL_TYPE_FUNCTION";
            break;
        case SYMBOL_TYPE_VARIABLE:
            return "SYMBOL_TYPE_VARIABLE";
            break;
        case SYMBOL_TYPE_STRUCTURE:
            return "SYMBOL_TYPE_STRUCTURE";
            break;
        case SYMBOL_TYPE_MEMBER:
            return "SYMBOL_TYPE_MEMBER";
            break;
        case SYMBOL_TYPE_UNKNOWN:
        default:
            return "SYMBOL_TYPE_UNKNOWN";
            break;
    }
}

void print_binding(const binding_t *b) {
    if (b != NULL) {
        printf("Name: %s | ", b->name);
        printf("Symbol Type: %s | ", sym_type_to_str(b->symbol_type));

        switch (b->symbol_type) {
            case SYMBOL_TYPE_FUNCTION:
                printf("Return Type: %s | ", type_to_str(b->data.function_type.return_type));
                printf("Struct Type: %s | ", b->data.function_type.struct_type);
                printf("IsStructType?: %d | ", b->data.function_type.is_struct_type);
                printf("IsArrayType?: %d | ", b->data.function_type.is_array_type);
                printf("Num Dimensions: %d | ", b->data.function_type.num_dimensions);
                printf("Num Args: %d\n", b->data.function_type.num_args);
                break;
            case SYMBOL_TYPE_VARIABLE:
                printf("Data Type: %s | ", type_to_str(b->data.variable_type.type));
                printf("Struct Type: %s | ", b->data.variable_type.struct_type);
                printf("IsStructType?: %d | ", b->data.variable_type.is_struct_type);
                printf("IsArrayType?: %d | ", b->data.variable_type.is_array_type);
                printf("Num Dimensions: %d\n", b->data.variable_type.num_dimensions);
                break;
            case SYMBOL_TYPE_STRUCTURE:
                printf("Struct Type: %s | ", b->data.structure_type.struct_type);
                printf("Num Members: %d\n", b->data.structure_type.num_members);
                break;
            case SYMBOL_TYPE_MEMBER:
                printf("Data Type: %s | ", type_to_str(b->data.member_type.type));
                printf("Struct Type: %s | ", b->data.member_type.struct_type);
                printf("IsStructType?: %d | ", b->data.member_type.is_struct_type);
                printf("IsArrayType?: %d | ", b->data.member_type.is_array_type);
                printf("Num Dimensions: %d\n", b->data.member_type.num_dimensions);
                break;
            case SYMBOL_TYPE_UNKNOWN:
            default:
                printf("Unknown Symbol Type\n");
                break;
        }
    }
}

void print_table(const symtab_t *st) {
    if (st != NULL) {
        hashtable *ht = st->table;

        if (ht == NULL) {
            log_error("Unable to access symbol table for printing!");
        }

        // Iterate over hash table and print each key/value pair
        printf("Symbol Table: Level %d\n", st->level);
        printf("==================================================================================="
               "=====================================================================\n");
        for (int idx = 0; idx < MAX_SLOTS; idx++) {
            if (vector_length(ht->slots[idx]) <= 0) {
                continue;
            } else {
                vector *vec = ht->slots[idx];

                vecnode *vn = vec->head;

                while (vn != NULL) {
                    binding_t *b = (binding_t *)vn->data;
                    print_binding(b);

                    vn = vn->next;
                }
            }
        }
        printf("==================================================================================="
               "=====================================================================\n");
    }
}

symtab_t *symtab_new() {
    symtab_t *new = (symtab_t *)malloc(sizeof(symtab_t) + 1);

    if (new != NULL) {
        memset(new, 0, sizeof(symtab_t));
    } else {
        log_error("Unable to allocate symtab_t");
    }

    hashtable *ht = mk_hashtable();

    if (ht != NULL) {
        new->table = ht;
    } else {
        log_error("Unable to allocate hash table for use in a symbol table");
    }

    return new;
}

void symtab_free(symtab_t *st) {
    if (st != NULL) {
        ht_free(st->table);
        free(st);
    }
}
