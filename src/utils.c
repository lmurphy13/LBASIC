/**
 * Utilities Module
 * File: utils.c
 * Author: Liam M. Murphy
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "utils.h"

vector *mk_vector() {
    vector *retval = NULL;

    retval = (vector *)malloc(sizeof(vector));

    if (retval == NULL) {
        log_error("Unable to allocate new vector");
        return retval;
    }

    memset(retval, 0, sizeof(retval));
    retval->head  = NULL;
    retval->tail  = NULL;
    retval->count = 0;

    return retval;
}

void vector_free(vector **vec) {
    if (vec != NULL) {
        if (vector_length(*vec) > 0) {
            vecnode *curr = (*vec)->head;

            while (curr != NULL) {
                vecnode *next = curr->next;
                free(curr);
                curr = next;
            }
        }

        free(*vec);
        *vec = NULL;
    }
}

void vector_add(vector *vec, void *data) {
    if (vec != NULL) {
        if (data != NULL) {
            vecnode *node = malloc(sizeof(vecnode));
            memset(node, 0, sizeof(node));

            node->data = data;
            node->next = NULL;

            if (vec->head == NULL) {
                vec->head = node;
                vec->tail = node;
            } else {
                vec->tail->next = node;
                vec->tail       = node;
            }

            vec->count++;

            printf("added new vector element\n");

        } else {
            log_error("Cannot add NULL data to vector");
            exit(1);
        }
    } else {
        log_error("Cannot add element to NULL vector!");
        exit(1);
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
                vecnode *old_head = vec->head;

                node->next = old_head;
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
        if (vector_length(vec) == 0) {
            log_error("Cannot pop from empty vector");
        } else if (vector_length(vec) == 1) {
            if (vec->head != NULL) {
                free(vec->head);
                vec->head == NULL;
                vec->count--;
            }
        } else if (vector_length(vec) > 1) {
            vecnode *curr = vec->head;

            // Find the element before the tail
            while (curr != NULL) {
                if (curr->next->next == NULL) {
                    break;
                }

                curr = curr->next;
            }

            // Sanity check that we are the penultimate node
            if (curr != NULL && curr->next != NULL && curr->next->next == NULL) {
                free(curr->next->next);

                curr->next = NULL;
                vec->tail  = curr;
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

vecnode *get_nth_node(vector *vec, const int n) {
    vecnode *retval = NULL;

    if (vec != NULL) {
        const int length = vector_length(vec);
        if (n > length) {
            char msg[128] = {'\0'};
            snprintf(msg, sizeof(msg), "Cannot get node %d from a vector with length %d", n,
                     length);

            log_error(msg);
        } else {
            vecnode *vn = vec->head;
            int index   = 1;
            while (index < n) {
                if (vn != NULL) {
                    if (n == 1) {
                        retval = vec->head;
                        break;
                    }

                    vn = vn->next;
                    index++;
                }
            }

            retval = vn;
        }
    }

    return retval;
}
