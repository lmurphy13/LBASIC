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
        exit(1);
    }

    retval->data = NULL;
    retval->next = NULL;

    return retval;
}

void vector_push(vector *vec, void *data) {
    vector *ptr = vec;

    if (ptr != NULL) {
        if (ptr->next == NULL) {
            ptr->next = mk_vector();

            if (ptr->next != NULL) {
                ptr->next->data = data;
            }

        } else {
            while (ptr->next != NULL) {
                ptr = ptr->next;
            }

            ptr->next = mk_vector();

            if (ptr->next != NULL) {
                ptr->next->data = data;
            }
        }
    }
}

void vector_pop(vector *vec) {
    vector *ptr = vec;

    if (ptr != NULL) {
        while (ptr->next != NULL) {
            ptr = ptr->next;
        }
    }
}

vector *vector_next(vector *vec) {
    if (vec == NULL) {
        return NULL;
    } else {
        return vec->next;
    }
}
