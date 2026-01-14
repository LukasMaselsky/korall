#include "hashmap.h"


static inline uint32_t hash(char* key, size_t length) {
    // FNV-1a
    uint32_t hash = 2166136261u;
    for (unsigned char c = *key; c != '\0'; c = *(++key)) {
        hash ^= c;
        hash *= 16777619;
    }
    return hash % length;
}

void hashmap_init(HashMap* hmap, size_t capacity) {
    
}


void hashmap_add(HashMap *hmap, char *item) {
	uint32_t index = hash(item, hmap->capacity);
    printf("Index: %u\n", index);
}

int hashmap_get(HashMap* hmap, char* item) {
    uint32_t index = hash(item, hmap->capacity);
    return 0;
}