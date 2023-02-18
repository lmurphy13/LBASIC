/**
 * Utilities Module Public Definitions
 * File: utils.h
 * Author: Liam M. Murphy
 */

#ifndef UTILS_H
#define UTILS_H

typedef struct vector {
    void *data;
    struct vector *next;
} vector;

// Allocate a new vector
vector *mk_vector(void);

// Add an element to the end of a vector
void vector_push(vector *vec, void *data);

// Remove the tail element from a vector
void vector_pop(vector *vec);

// Get the next vector element
vector *vector_next(vector *vec);

#endif // UTILS_H
