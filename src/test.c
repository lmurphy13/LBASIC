#if defined(DEBUG)

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

    char *liam = "Liam";
    char *is   = "is";
    char *the  = "the";
    char *best = "best";

    if (v != NULL) {
        vector_add(v, liam);
        vector_add(v, is);
        vector_add(v, the);
        vector_add(v, best);
    } else {
        printf("mk_vector failed!\n");
    }

    // Iterate over vector
    vecnode *curr = v->head;
    while (curr != NULL) {
        printf("%s\n", curr->data);

        curr = curr->next;
    }

    printf("\n\n");

    char *blah = "blah";
    vector_prepend(v, blah);

    char *blah2 = "blahBlah";
    vector_prepend(v, blah2);

    // Iterate over vector
    curr = v->head;
    while (curr != NULL) {
        printf("%s\n", curr->data);

        curr = curr->next;
    }

    printf("\n\n");

    v->tail->data = NULL;
    vector_pop(v);

    // Iterate over vector
    curr = v->head;
    while (curr != NULL) {
        if (curr->data != NULL) {
            printf("%s\n", curr->data);
        } else {
            printf("NULL\n");
        }

        curr = curr->next;
    }
}

#endif
