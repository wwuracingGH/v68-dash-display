#ifndef HASHMAP_H_
#define HASHMAP_H_
#include <linux/can.h>

/* Nicole Swierstra
 *
 * I stole this from 2025 nicole
 */

/* resizeable array */
typedef struct vector {
    int ptr;
    int size;
    char** arr;
} vector_t;

/* init and free the data */
void vector_init(vector_t* vector);
void vector_free(vector_t* vector);

/* adds an item to the end of the vector type */
void vector_push(vector_t* vector, char* str);

/* removes the last item from the vector and returns it */
char* vector_popl(vector_t* vector);
/* removes the first item from the vector and returns it */
char* vector_popf(vector_t* vector);

/* inserts an item into the vector */
void vector_insert(vector_t* vector, char* str, int location);
/* resizes the vector */
void vector_resize(vector_t* vector, int newsize);

typedef struct hashPair {
    int key;
    void (*func)(struct can_frame);
    struct hashPair* next;
} hashPair_t;

typedef struct hashMap {
    int size;
    int items;
    hashPair_t** arr;
} hashMap_t;

/* inits and frees the hashmap */
void hashMap_init(hashMap_t* hashMap);
void hashMap_free(hashMap_t* hashMap);

/* returns the value of the corresponding key */
void (*hashMap_get(hashMap_t* hashMap, int key))(struct can_frame);
/* pushes a value to the hashmap and returns the last value stored */
void hashMap_put(hashMap_t* hashMap, int key, void (*func)(struct can_frame));
/* grows the hashmap */
void hashMap_grow(hashMap_t* hashMap);

/* prints the hashmap contents out to the console */
void hashMap_dump(hashMap_t* hashMap);
/* prints the memory locations of the hashpairs out to the console */
void hashMap_adrdump(hashMap_t* hashMap);

#endif /* HASHMAP_H_ */