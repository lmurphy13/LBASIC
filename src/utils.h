/**
 * Utilities Module Public Definitions
 * File: utils.h
 * Author: Liam M. Murphy
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

#define MAX_LINE 4096
#define MAX_SLOTS 1024

/* Vectors */
typedef struct vecnode {
    void *data;
    struct vecnode *next;
} vecnode;

typedef struct vector {
    vecnode *head;
    vecnode *tail;
    int count;
} vector;

typedef struct hashtable {
    vector *slots[MAX_SLOTS];
} hashtable;

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

/* Hash Table */
// Allocate a new hash table
hashtable *mk_hashtable(void);

// Free a hash table
void ht_free(hashtable *ht);

// Insert an element
void ht_insert(hashtable *ht, void *key, void *data);

// Lookup an element
void *ht_lookup(hashtable *ht, void *key, bool (*ht_compare)(vecnode *vn, void *key));

// Remove an element
void ht_remove(hashtable *ht, void *key);

#endif // UTILS_H
