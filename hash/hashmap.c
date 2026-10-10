#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "hash/hashmap.h"

void vector_init(vector_t* vector){
    vector->ptr = 0;
    vector->size = 64;
    vector->arr = (char**)malloc(sizeof(char*) * vector->size);
}

void vector_free(vector_t* vector){
    for (int i = 0; i < vector->ptr; i++){
        free(vector->arr[i]);
    }
    free(vector->arr);
    vector->ptr = 0;
    vector->size = 0;
}

void vector_push(vector_t* vector, char* str){
    vector->arr[vector->ptr] = strdup(str);
    vector->ptr++;
    if (vector->ptr >= vector->size) {
        vector->size *= 2;
        vector->arr = (char**)realloc(vector->arr, vector->size);
    }
}

char* vector_popl(vector_t* vector){
    if (vector->ptr == 0) return NULL;
    vector->ptr--;
    return vector->arr[vector->ptr];
}

/* not very fast lol */
char* vector_popf(vector_t* vector){
    if (vector->ptr == 0) return NULL;
    char* c = vector->arr[0];
    for (int i = 1; i < vector->ptr; i++) {
        vector->arr[i - 1] = vector->arr[i];
    }
    vector->ptr--;
    return c;
}

void vector_insert(vector_t* vector, char* str, int location){
    vector_push(vector, str);
    char* newstr = vector->arr[vector->ptr-1];
    for (int i = vector->ptr - 1; i > location; i--){
        vector->arr[i] = vector->arr[i-1];
    }
    vector->arr[location] = newstr;
}

void vector_resize(vector_t* vector, int newsize){
    vector->size = 2 << __builtin_clz(newsize); /* evil code */
    vector->arr = (char**)realloc(vector->arr, vector->size);
    for (; vector->ptr < newsize; vector->ptr++){
        vector->arr[vector->ptr] = NULL;
    }
}

/* creates and returns a new hashpair */
hashPair_t* hashPair_new(int key, void (*handler)(struct can_frame), hashPair_t* next) {
    hashPair_t* pair = (hashPair_t*)malloc(sizeof(hashPair_t));

    pair->key = key;
    pair->func = handler;
    pair->next = next;

    return pair;
}

hashPair_t* hashPair_get(hashPair_t* pair, int key){
    if (pair == NULL || pair->key == key) return pair;
    return hashPair_get(pair->next, key);
}

/* deletes a hashPair */
void hashPair_delete(hashPair_t** pair){
    free((*pair)->key);
    free((*pair));
    (*pair) = NULL;
}

void hashPair_deleteAll(hashPair_t* start){
    if (start == NULL) return;
    hashPair_deleteAll(start->next);
    hashPair_delete(&start);
}

void hashPair_dump(hashPair_t* pair){
    if (pair == NULL){
        printf("null\n");
    } else {
        printf("(%s, %d)->", pair->key, pair->func);
        hashPair_dump(pair->next);
    }
}

void hashMap_init(hashMap_t* hashMap){
    hashMap->size = 16;
    hashMap->items = 0;
    hashMap->arr = (hashPair_t**)malloc(sizeof(hashPair_t*) * hashMap->size);

    for (int i = 0; i < hashMap->size; i++){
        hashMap->arr[i] = 0;
    }
}

void hashMap_free(hashMap_t* hashMap){
    for (int i = 0; i < hashMap->size; i++) {
        hashPair_deleteAll(hashMap->arr[i]);
    }
    free(hashMap->arr);
}

void (*hashMap_get(hashMap_t* hashMap, int key))(struct can_frame){
    int hash = key % hashMap->size;
    hashPair_t* p = hashPair_get(hashMap->arr[hash], key);
    return p == NULL ? NULL : p->func;
}

void hashMap_put(hashMap_t* hashMap, int key, void (*func)(struct can_frame)){
    int hash = key % hashMap->size;

    hashPair_t* old = hashPair_get(hashMap->arr[hash], key);
    if (old != NULL){
        void (*oldfunc)(struct can_frame)  = old->func;
        old->func = func;
    }

    hashMap->arr[hash] = hashPair_new(key, func, hashMap->arr[hash]);
    hashMap->items++;

    hashMap_grow(hashMap);
    return NULL;
}

void hashMap_grow(hashMap_t* hashMap){
    int oldsize = hashMap->size;
    if (hashMap->items <= (int)(0.8f * oldsize)) return;

    hashMap->size *= 2;
    hashPair_t** ob = hashMap->arr;
    hashMap->arr = (hashPair_t**)malloc(sizeof(hashPair_t*) * hashMap->size);
    for (int i = 0; i < hashMap->size; i++){
        hashMap->arr[i] = 0;
    }

    hashMap->items = 0;
    for (int i = 0; i < oldsize; i++){
        for (hashPair_t* n = ob[i]; n != NULL; n = n->next){
            hashMap_put(hashMap, n->key, n->func);
        }
        hashPair_deleteAll(ob[i]);
    }
}

void hashMap_dump(hashMap_t* hashMap){
    for (int i = 0; i < hashMap->size; i++){
        hashPair_dump(hashMap->arr[i]);
    }
}

void hashMap_adrdump(hashMap_t* hashMap){
    for (int i = 0; i < hashMap->size; i++){
        printf("%lX\n", (size_t)(hashMap->arr[i]));
    }
}
