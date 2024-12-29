/**
 * Vector Utility Public Definitions
 * File: vector.h
 * Author: Liam M. Murphy
 */

#ifndef VECTOR_H
#define VECTOR_H

#define MAX_LINE 4096

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

// Free a vector
void vector_free(vector **vec);

// Add an element to the end of a vector
void vector_add(vector *vec, void *data);

// Add an element to the head of a vector
void vector_prepend(vector *vec, void *data);

// Remove the tail element from a vector
void vector_pop(vector *vec);

// Remove the head element from a vector and return it
vecnode *vector_pop_head(vector *vec);

// Return a pointer to the vecnode at vec->head
vecnode *vector_top(vector *vec);

// Get length of vector
int vector_length(vector *vec);

// Get the nth node from a vector
vecnode *get_nth_node(vector *vec, const int n);

/* Line Map (vector)
 *
 *  The idea here is to maintain a list of strings for each line in the
 *  input program. This will be used to display more effective error
 *  messages in the front-half of the compiler. */
typedef struct line_s {
    char text[MAX_LINE];
} line_t;


#endif