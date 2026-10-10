#include <stdio.h>
#include <stdlib.h>
#include "hash/hashset.h"

void hashSet_init(hashSet_t* hashSet){
    hashSet->size = 16;
    hashSet->items = 0;
    hashSet->arr = (int32_t*)malloc(sizeof(uint*) * hashSet->size);

    for (int i = 0; i < hashSet->size; i++){
        hashSet->arr[i] = -1;
    }
}

void hashSet_free(hashSet_t* hashSet){
    free(hashSet->arr);
}

int hashSet_get(hashSet_t* hashSet, int key) {
    int hash = key % hashSet->size;
    
    while(hashSet->arr[hash] != -1) {
        if (hashSet->arr[hash] == key) return 1;
        hash++;
    }

    return 0;
}

void hashSet_put(hashSet_t* hashSet, int key){
    int hash = key % hashSet->size;

    while(hashSet->arr[hash] != -1) {
        if (hashSet->arr[hash] == key) return; /* no double insertions */
        hash++;
    }

    hashSet->arr[hash] = key;

    hashSet_grow(hashSet);
}

void hashSet_grow(hashSet_t* hashSet){
    int oldsize = hashSet->size;
    if (hashSet->items <= (int)(0.8f * oldsize)) return;

    hashSet->size *= 2;
    int32_t* ob = hashSet->arr;
    
    hashSet->arr = (int32_t*)malloc(sizeof(int32_t) * hashSet->size);
    for (int i = 0; i < hashSet->size; i++){
        hashSet->arr[i] = -1;
    }

    hashSet->items = 0;
    for (int i = 0; i < oldsize; i++){
        if (ob[i] != -1)
            hashSet_put(hashSet, ob[i]);
    }

    free(ob);
}

void hashSet_dump(hashSet_t* hashSet){
    for (int i = 0; i < hashSet->size; i++){
        if (hashSet->arr[i] != -1)
            printf("0x%X\n", hashSet->arr[i]);
    }
}
