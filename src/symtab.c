/**
 * LBASIC Symbol Table Module
 * File: symtab.h
 * Author: Liam M. Murphy
 */

#include "symtab.h"
#include "error.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Compare bindings by name
bool ht_compare_binding(vecnode *vn, void *key) {
    bool retval = false;

    binding_t *b = (binding_t *)vn->data;
    retval       = (strcmp(b->name, (char *)key) == 0);

    return retval;
}

/* Symbol table interface */
symtab_t *symtab_new(void) {
    symtab_t *retval = (symtab_t *)calloc(1, sizeof(symtab_t));

    if (retval != NULL) {
        retval->scope = 0;
        retval->table = mk_hashtable();
        retval->prev = NULL;
        retval->next = NULL;

        if (retval->table == NULL) {
            log_error("Failed to allocate hash table");
        }
    }

    return retval;
}

void symtab_insert(symtab_t *st, binding_t *binding) {
    if (st != NULL) {
        ht_insert(st->table, binding->name, binding);
    } else {
        log_error("Symbol table is NULL");
    }
}

binding_t *symtab_lookup(symtab_t *st, char *identifier) {
    binding_t *retval = NULL;

    if (st != NULL) {
        retval = ht_lookup(st->table, (char *)identifier, ht_compare_binding);
    } else {
        log_error("Symbol table is NULL");
    }

    return retval;
}

void symtab_free(symtab_t *st) { assert(false && "Not implemented"); }

binding_t *mk_binding(symbol_type_t symbol_type) {
    binding_t *retval = (binding_t *)calloc(1, sizeof(binding_t));

    if (NULL != retval) {
        retval->symbol_type = symbol_type;
    }

    return retval;
}


// NAME     SYMBOL TYPE     DATA TYPE       ETC
void print_binding(const binding_t *binding) {
    if (NULL != binding) {
        switch (binding->symbol_type) {
            case SYMBOL_TYPE_FUNCTION:
                debug("none yet");
                break;
            case SYMBOL_TYPE_VARIABLE:
                printf("%s\tVARIABLE\t%s\tis_array: %d (dimensions=%d)\tis_struct: %d (struct_type='%s')\n",
                    binding->name, type_to_str(binding->data.variable_type.type),
                    binding->data.variable_type.is_array_type,
                    binding->data.variable_type.num_dimensions, binding->data.variable_type.is_struct_type,
                    binding->data.variable_type.struct_type);
                break;
            case SYMBOL_TYPE_STRUCTURE:
                debug("none yet");
                break;
            case SYMBOL_TYPE_MEMBER:
                debug("none yet");
                break;
            case SYMBOL_TYPE_UNKNOWN:
            default:
                log_error("Unknown binding type (type=%d)", binding->symbol_type);
        }
    }
}

void print_symbol_table(const symtab_t *st) {
    if (st->scope != 0) {
        log_error("%s() must be called on scope 0", __FUNCTION__);
    }

    printf("NAME\tSYMBOL TYPE\tDATA TYPE\tETC.\n");
    printf("========================================================================================\n");
    printf("Scope: %d\n", st->scope);
    for (int row = 0; row < MAX_SLOTS; row++) {
        if (NULL != st->table->slots[row]) {
            if (st->table->slots[row]->count > 1) {
                debug("Table slot %d has more than one entry", row);
            } else {
                const binding_t *b = (binding_t *)st->table->slots[row]->head->data;
                print_binding(b);
            }
        }
    }
    printf("========================================================================================\n");

    symtab_t *tab = st->next;
    while (NULL != tab) {
        printf("========================================================================================\n");
        printf("Scope: %d\n", tab->scope);
        for (int row = 0; row < MAX_SLOTS; row++) {
            if (NULL != tab->table->slots[row]) {
                if (tab->table->slots[row]->count > 1) {
                    debug("Table slot %d has more than one entry", row);
                } else {
                    const binding_t *b = (binding_t *)tab->table->slots[row]->head->data;
                    print_binding(b);
                }
            }
        }
        printf("========================================================================================\n");

        tab = tab->next;
    }
}