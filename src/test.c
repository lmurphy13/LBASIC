/**
 * Internal Test Suite
 * File: test.c
 * Author: Liam M. Murphy
 */

#if defined(DEBUG)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test.h"
#include "utils.h"

static void print_header() { printf("Running internal tests.......\n"); }

static void print_string_vec(vector *v) {
    if (v != NULL) {
        vecnode *curr = v->head;

        printf("Length: %d\n", vector_length(v));

        if (vector_length(v) > 0) {
            while (curr != NULL) {
                printf("%s\n", curr->data);

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

    for (int i = 0; i < 5; i++) {
        printf("popping tail\n");
        vector_pop(v);
        print_string_vec(v);
    }

    printf("freeing vector\n");
    vector_free(&v);

    print_string_vec(v);
}

#endif
