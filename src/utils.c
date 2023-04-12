#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "utils.h"

vector *mk_vector() {
    vector *retval = NULL;

    retval = (vector *)malloc(sizeof(vector));
    memset(retval, 0, sizeof(retval));

    if (retval == NULL) {
        log_error("Unable to allocate new vector");
    }

    retval->head  = NULL;
    retval->tail  = NULL;
    retval->count = 0;

    return retval;
}

void vector_add(vector *vec, void *data) {
    if (vec != NULL) {
        if (data != NULL) {
            vecnode *node = malloc(sizeof(vecnode));
            memset(node, 0, sizeof(node));

            node->data = data;

            if (vec->head == NULL) {
                vec->head = node;
                vec->tail = node;
            } else {
                vec->tail->next = node;
                vec->tail       = node;
            }

            vec->count++;

        } else {
            log_error("Cannot add NULL data to vector");
        }
    } else {
        log_error("Cannot add element to NULL vector!");
    }
}

void vector_prepend(vector *vec, void *data) {
    if (vec != NULL) {
        if (data != NULL) {
            vecnode *node = malloc(sizeof(vecnode));
            memset(node, 0, sizeof(node));

            node->data = data;

            if (vec->head == NULL) {
                vec->head = node;
                vec->tail = node;
            } else {
                node->next = vec->head;
                vec->head  = node;
            }

            vec->count++;
        } else {
            log_error("Cannot add NULL data to vector");
        }
    } else {
        log_error("Cannot add element to NULL vector");
    }
}

/*
 * Removes the tail of the vector. Data pointed to within the node must be freed
 * from within the caller.
 */
void vector_pop(vector *vec) {
    if (vec != NULL) {

        if (vector_length(vec) <= 0) {
            log_error("Cannot pop from empty vector");
            return;
        }

        vecnode *iter = vec->head;

        // Find element before tail
        while (iter->next->next != NULL) {
            // seek
            iter = iter->next;
        }

        // Sanity check
        if (iter->next != NULL && iter->next->next == NULL) {

            if (vec->tail != NULL) {
                free(vec->tail);
                vec->tail = iter;
                vec->count--;
            } else {
                log_error("Cannot free a NULL vector tail");
            }
        }
    }
}

int vector_length(vector *vec) {
    if (vec != NULL) {
        return vec->count;
    }
}
