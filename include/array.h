#ifndef ARRAY_H
#define ARRAY_H
#include "utils.h"

typedef struct {
    const void* data;
    size_t element_size;
    size_t capacity;
    size_t size;
} Array;

void array_init(Array* arr, void* buffer, size_t elem_size, size_t capacity);

bool array_add(Array* arr, const void* element);

void* array_get(Array* arr, size_t index);


#endif