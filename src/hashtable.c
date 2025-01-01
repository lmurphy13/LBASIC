#include "hashtable.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"

// Allocate a new hash table
hashtable *ht_new() {
    hashtable *retval = NULL;

    retval = (hashtable *)calloc(1, sizeof(hashtable));

    return retval;
}

void ht_free(hashtable **ht) {
    if (ht != NULL && *ht != NULL) {
        for (int slot = 0; slot < MAX_SLOTS; slot++) {
            if ((*ht)->slots[slot] != NULL) {
                vector_free(&(*ht)->slots[slot]);
            }
        }
        free(*ht);
        *ht = NULL;
    }
}

// FNV-1a hash algorithm from https://craftinginterpreters.com/hash-tables.html
static uint32_t ht_hash_FNV_1a(void *key) {
    const char *const charkey = (char *)key;

    const size_t length = strlen(charkey);

    // debug("Length: %d", length);

    uint32_t hash = 2166136261U;
    for (int i = 0; i < length; i++) {
        hash ^= (uint8_t)charkey[i];
        hash *= 16777619;
    }

    // debug("Hash: %lu", hash);

    return hash;
}

// Insert an element
void ht_insert(hashtable *ht, void *key, void *data) {
    if (ht != NULL) {
        if (key != NULL) {
            if (data != NULL) {
                const unsigned int index = ht_hash_FNV_1a(key) % MAX_SLOTS;
                // printf("INDEX INSERT: %u (key=%s)\n", index, (char *)key);

                if (ht->slots[index] == NULL) {
                    ht->slots[index] = mk_vector();
                    if (ht->slots[index] != NULL) {
                        vector_add(ht->slots[index], data);
                        ht->num_values++;
                    } else {
                        log_error("Unable to create vector for hashtable insertion at index: %d",
                                  index);
                    }
                } else {
                    // Hash index collision, so append to vector (buckets 'n chaining)
                    vector_add(ht->slots[index], data);
                    ht->num_values++;
                }
            } else {
                log_error("Unable to access data for insertion");
            }
        } else {
            log_error("Unable to access key for insertion");
        }
    } else {
        log_error("Unable to access hashtable for insertion");
    }
}

// Lookup an element
void *ht_lookup(hashtable *ht, void *key, bool (*ht_compare)(vecnode *vn, void *key)) {
    void *retval = NULL;

    if (ht != NULL) {
        if (key != NULL) {
            const unsigned int index = ht_hash_FNV_1a(key) % MAX_SLOTS;
            // debug("INDEX LOOKUP: %u", index);

            vector *slot_ptr = ht->slots[index];
            if (slot_ptr != NULL) {
                if (slot_ptr->count == 1) {
                    // printf("here!\n");
                    vecnode *vn = slot_ptr->head;
                    if (vn != NULL) {
                        retval = vn->data;
                    } else {
                        log_error("Unable to access vecnode at vector head for lookup");
                    }
                } else {
                    // Need to check each node in the vector for a match
                    vecnode *vn = slot_ptr->head;
                    if (vn != NULL) {
                        while (vn != NULL) {
                            // Use comparison callback to become generic
                            if ((*ht_compare)(vn, key)) {
                                retval = vn->data;
                                break;
                            }
                            vn = vn->next;
                        }
                    }
                }
            } else {
                // debug("Vector does not exist at index %d\n", index);
            }
        } else {
            log_error("Unable to access key for lookup");
        }
    } else {
        log_error("Unable to access hashtable for lookup");
    }

    return retval;
}

void ht_print(hashtable *ht) {
    if (NULL != ht) {
        for (int slot = 0; slot < MAX_SLOTS; slot++) {
            if (NULL == ht->slots[slot]) {
                printf("Slot %d empty.\n", slot);
            } else {
                printf("Slot %d\t\tData: ", slot);
                // Walk the linked list
                vecnode *vn = ht->slots[slot]->head;
                while (NULL != vn) {
                    if (NULL != vn->data) {
                        printf("%s -> ", (char *)vn->data);
                    }
                    vn = vn->next;
                }
                printf("NULL\n");
            }
        }
    }
}

// Remove an element
// void ht_remove(hashtable *ht, void *key);
