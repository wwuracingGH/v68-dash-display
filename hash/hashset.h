#ifndef _HASHSET_H_
#define _HASHSET_H_
#include <stdint.h>

/*
 * Nicole Swierstra
 *
 * Implementation of a hashset using open addressing
 * allows for positive integers only
 */

typedef struct hashset {
    int size;
    int items;
    int32_t* arr;
} hashSet_t;

/* inits and frees the hashset */
void hashSet_init(hashSet_t* hashSet);
void hashSet_free(hashSet_t* hashSet);

/* returns 1 if the item is in the set, 0 if not */
int hashSet_get(hashSet_t* hashSet, int key);
/* puts an item into the set */
void hashSet_put(hashSet_t* hashSet, int key);
/* removes an item from the set */
void hashSet_remove(hashSet_t* hashSet, int key);

/* grows the hashset */
void hashSet_grow(hashSet_t* hashSet);

/* prints the hashset contents out to the console - debug function */
void hashSet_dump(hashSet_t* hashSet);

#endif
