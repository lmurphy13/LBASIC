/**
 * Utilities Module Public Definitions
 * File: utils.h
 * Author: Liam M. Murphy
 */

#ifndef UTILS_H
#define UTILS_H

typedef struct vecnode {
    void *data;
    struct vecnode *next;
} vecnode;

typedef struct vector {
    vecnode *head;
    vecnode *tail;
    int count;
} vector;

// Allocate a new vector
vector *mk_vector(void);

// Add an element to the end of a vector
void vector_add(vector *vec, void *data);

// Add an element to the head of a vector
void vector_prepend(vector *vec, void *data);

// Remove the tail element from a vector
void vector_pop(vector *vec);

// Get length of vector
int vector_length(vector *vec);

#endif // UTILS_H
