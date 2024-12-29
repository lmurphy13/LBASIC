/**
 * Vector Utility Module
 * File: vector.c
 * Author: Liam M. Murphy
 */

#include "vector.h"

#include "error.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

vector *mk_vector() {
    vector *retval = NULL;

    retval = (vector *)calloc(1, sizeof(vector));

    if (retval == NULL) {
        log_error("Unable to allocate new vector");
        return retval;
    }

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

                if (curr->data != NULL) {
                    free(curr->data);
                }

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
            vecnode *node = calloc(1, sizeof(vecnode));

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

            //debug("added new vector element");

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
            vecnode *node = calloc(1, sizeof(vecnode));
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
                vec->head = NULL;
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

/* Pop the head from a vector and return a pointer to it */
vecnode *vector_pop_head(vector *vec) {
    vecnode *retval = NULL;

    if (vec != NULL) {
        if (vector_length(vec) == 0) {
            // log_error("Cannot pop from empty vector");
            printf("Cannot pop from empty stack\n");
        } else if (vector_length(vec) == 1) {
            if (vec->head != NULL) {
                retval    = vec->head;
                vec->head = NULL;
                vec->count--;
            }
        } else if (vector_length(vec) > 1) {
            // Save a pointer to the current head
            vecnode *curr_head = vec->head;
            retval             = vec->head;

            // Reassign the head pointer to the next node
            if (curr_head->next != NULL) {
                vec->head = curr_head->next;
                vec->count--;
            } else {
                log_error("Cannot assign new head node to a NULL element");
            }
        }
    }

    return retval;
}

vecnode *vector_top(vector *vec) {
    vecnode *retval = NULL;

    if (vec != NULL) {
        if (vec->head != NULL) {
            retval = vec->head;
        }
    }

    return retval;
}

int vector_length(vector *vec) {
    int retval = 0;

    if (vec != NULL) {
        retval = vec->count;
    }

    return retval;
}

vecnode *get_nth_node(vector *vec, const int n) {
    vecnode *retval = NULL;

    if (vec != NULL) {
        const int length = vector_length(vec);
        if (n > length) {
            log_error("Cannot get node %d from a vector with length %d", n, length);
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