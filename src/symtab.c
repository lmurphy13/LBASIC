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
        retval->level = 0;
        retval->table = ht_new();
        retval->prev  = NULL;
        retval->next  = NULL;

        if (retval->table == NULL) {
            log_error("Failed to allocate hash table");
        }
    }

    return retval;
}

void symtab_insert(symtab_t *st, binding_t *binding) {
    if (st != NULL) {
        ht_insert(st->table, binding->name, binding);

        debug("Added '%s' (symbol type %d) to scope level %d", binding->name, binding->symbol_type,
              st->level);
    } else {
        log_error("Symbol table is NULL");
    }
}

binding_t *symtab_lookup(symtab_t *scope, char *identifier, bool single_scope) {
    binding_t *retval = NULL;

    if (scope != NULL) {
        debug("Looking for '%s' within scope level %d (name='%s')", identifier, scope->level,
              scope->name);
        retval = ht_lookup(scope->table, (char *)identifier, ht_compare_binding);

        // Nothing found
        if (NULL == retval) {
            // If single_scope is true, we are limiting our search to the current scope only
            if (!single_scope) {
                // Otherwise, potentially search parent scopes
                if (scope->prev == NULL) {
                    // If we are already in the global scope, we didn't find anything
                    return NULL;
                } else {
                    // If the identifier isn't found in the current scope, look in its parent
                    retval = symtab_lookup(scope->prev, identifier, single_scope);
                }
            }
        }
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

static void formals_to_str(vector *formals, char *str) {
    if (NULL != formals) {
        size_t offset = 0;

        vecnode *vn = formals->head;
        while (NULL != vn) {
            node *f = (node *)vn->data;
            if (NULL != f) {
                char formal_str[4096] = {'\0'};
                // Build string
                snprintf(formal_str, 4096,
                         "\t\tName: %s\tType: %s\tis_array: %d (dimensions=%d)\tis_struct: %d "
                         "(struct_type='%s')\n",
                         f->data.formal.name, type_to_str(f->data.formal.type),
                         f->data.formal.is_array, f->data.formal.num_dimensions,
                         f->data.formal.is_struct, f->data.formal.struct_type);

                // Copy into str
                strncpy(str + offset, formal_str, strlen(formal_str));
                offset += strlen(formal_str);

                vn = vn->next;
            }
        }
    }
}

// NAME     SYMBOL TYPE     DATA TYPE       ETC
void print_binding(const binding_t *binding) {
    if (NULL != binding) {
        switch (binding->symbol_type) {
            case SYMBOL_TYPE_FUNCTION:
                char formals_str[8192] = {'\0'};
                formals_to_str(binding->data.function_type.formals, formals_str);

                printf("%s\tFUNCTION\t%s\tis_array: %d (dimensions=%d)\tis_struct: %d "
                       "(struct_type='%s')\n\tFormals (num_args: %d):\n%s",
                       binding->name, type_to_str(binding->data.function_type.return_type),
                       binding->data.function_type.is_array_type,
                       binding->data.function_type.num_dimensions,
                       binding->data.function_type.is_struct_type,
                       binding->data.function_type.struct_type,
                       binding->data.function_type.num_args, formals_str);
                break;
            case SYMBOL_TYPE_VARIABLE:
                printf("%s\tVARIABLE\t%s\tis_array: %d (dimensions=%d)\tis_struct: %d "
                       "(struct_type='%s')\n",
                       binding->name, type_to_str(binding->data.variable_type.type),
                       binding->data.variable_type.is_array_type,
                       binding->data.variable_type.num_dimensions,
                       binding->data.variable_type.is_struct_type,
                       binding->data.variable_type.struct_type);
                break;
            case SYMBOL_TYPE_FORMAL:
                printf("%s\tFORMAL\t%s\tis_array: %d (dimensions=%d)\tis_struct: %d "
                       "(struct_type='%s')\n",
                       binding->name, type_to_str(binding->data.variable_type.type),
                       binding->data.variable_type.is_array_type,
                       binding->data.variable_type.num_dimensions,
                       binding->data.variable_type.is_struct_type,
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
        printf("-----------------------------------------------------------------------------------"
               "-----------------\n");
    }
}

void print_symbol_table(const symtab_t *st) {
    if (st->level != 0) {
        log_error("%s() must be called on scope 0", __FUNCTION__);
    }

    printf("NAME\tSYMBOL TYPE\tDATA TYPE\tETC.\n");
    printf("======================================================================================="
           "=============\n");
    printf("Scope: %d (name='%s')\n", st->level, st->name);
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
    printf("======================================================================================="
           "=============\n\n\n");

    symtab_t *tab = st->next;
    while (NULL != tab) {
        printf("==================================================================================="
               "===="
               "=============\n");
        printf("Scope: %d (name='%s')\n", tab->level, tab->name);
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
        printf("==================================================================================="
               "===="
               "=============\n\n\n");

        tab = tab->next;
    }
}