/**
 * Internal Test Suite
 * File: test.c
 * Author: Liam M. Murphy
 */

#if defined(DEBUG)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symtab.h"
#include "test.h"
#include "utils.h"

static void print_header() { printf("Running internal tests.......\n"); }

static void print_string_vec(vector *v) {
    if (v != NULL) {
        vecnode *curr = v->head;

        printf("Length: %d\n", vector_length(v));

        if (vector_length(v) > 0) {
            while (curr != NULL) {
                printf("%s\n", (char *)curr->data);

                curr = curr->next;
            }
        }
        printf("==============\n");
    } else {
        printf("Cannot print NULL vector\n");
    }
}

void run_tests(void) {
    print_header();

    // Begin internal unit test environment below
    printf("Running vector tests.....\n");

    vector *v = mk_vector();

    char *val1 = "val1";
    char *val2 = "val2";
    char *val3 = "val3";
    char *val4 = "val4";

    if (v != NULL) {
        vector_add(v, val1);
        vector_add(v, val2);
        vector_add(v, val3);
        vector_add(v, val4);
    } else {
        printf("mk_vector failed!\n");
    }

    print_string_vec(v);

    char *val5 = (char *)malloc(sizeof(char) * 5);
    if (val5 != NULL) {
        snprintf(val5, 5, "%s", "val5");
    }

    vector_prepend(v, val5);

    print_string_vec(v);

    char *val6 = (char *)malloc(sizeof(char) * 5);
    if (val6 != NULL) {
        snprintf(val6, 5, "%s", "val6");
    }

    vector_prepend(v, val6);

    print_string_vec(v);

    printf("popping tail\n");
    vector_pop(v);
    print_string_vec(v);

    printf("popping tail\n");
    vector_pop(v);
    print_string_vec(v);

    printf("freeing vector\n");
    vector_free(&v);

    print_string_vec(v);

    printf("Running hashtable tests................\n");

    hashtable *ht = mk_hashtable();

    if (ht != NULL) {
        binding_t *b1 = mk_binding(SYMBOL_TYPE_VARIABLE);
        binding_t *b2 = mk_binding(SYMBOL_TYPE_VARIABLE);

        snprintf(b1->name, sizeof(b1->name), "%s", "LIAM");
        snprintf(b1->data.variable_type.struct_type, sizeof(b1->data.variable_type.struct_type),
                 "%s", "STRUCTYPE1");

        snprintf(b2->name, sizeof(b2->name), "%s", "MAIL");
        snprintf(b2->data.variable_type.struct_type, sizeof(b2->data.variable_type.struct_type),
                 "%s", "STRUCTYPE2");

        binding_t *b3                      = mk_binding(SYMBOL_TYPE_FUNCTION);
        b3->data.function_type.return_type = D_FLOAT;
        snprintf(b3->name, sizeof(b3->name), "%s", "BLAHBLAH");
        snprintf(b3->data.function_type.struct_type, sizeof(b3->data.function_type.struct_type),
                 "%s", "BLAHBLABHBLABHBLABHBLABH");

        ht_insert(ht, b1, b1);
        ht_insert(ht, b2, b2);
        ht_insert(ht, b3, b3);

        binding_t *lookup1 = (binding_t *)ht_lookup(ht, b2, ht_compare_binding);
        if (lookup1 != NULL) {
            print_binding(lookup1);
        }
    }
}

#endif
