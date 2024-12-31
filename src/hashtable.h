/**
 * Hash table Utility Public Definitions
 * File: vector.h
 * Author: Liam M. Murphy
 */

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "vector.h"
#include <stdbool.h>

#define MAX_SLOTS 1024

typedef struct hashtable {
    vector *slots[MAX_SLOTS];
    int num_values; // The sum of all elements within the table
} hashtable;

/* Hash Table */
// Allocate a new hash table
hashtable *ht_new(void);

// Free a hash table
void ht_free(hashtable **ht);

// Insert an element
void ht_insert(hashtable *ht, void *key, void *data);

// Lookup an element
void *ht_lookup(hashtable *ht, void *key, bool (*ht_compare)(vecnode *vn, void *key));

// Remove an element
void ht_remove(hashtable *ht, void *key);

// Print all elements in the table
void ht_print(hashtable *ht);

#endif