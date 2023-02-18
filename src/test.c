#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test.h"
#include "utils.h"

static void print_header() { printf("Running internal tests.......\n"); }

void run_tests(void) {
    print_header();

    // Begin internal unit test environment below

    vector *v = mk_vector();

    int *a = (int *)malloc(sizeof(int));
    *a = 5;

  
    if (v != NULL) {
        vector_push(v, a);
    } else {
        printf("mk_vector failed!\n");
    }

    while (v != NULL) {
        if (v->next != NULL) {
            if (v->data != NULL) {
                printf("v->data: %d\n", *(int *)v->data);
            } else {
                printf("v->data is null\n");
            }
        } else {
            printf("NULL\n");
        }

        v = vector_next(v);
    }
}
