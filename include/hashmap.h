#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
	char* key;
	int value;
} HashMapItem;

typedef struct {
	HashMapItem* data;
	size_t capacity;
} HashMap;

void hashmap_add(HashMap* hmap, char* item);

void hashmap_init(HashMap* hmap, size_t capacity);

int hashmap_get(HashMap* hmap, char* item);

#endif