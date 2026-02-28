#include "array.h"


void array_init(Array* arr, void* buffer, size_t elem_size, size_t capacity)
{
    arr->data = buffer;
    arr->element_size = elem_size;
    arr->capacity = capacity;
    arr->size = 0;
}

bool array_add(Array* arr, const void *element) {
    if (arr->size >= arr->capacity) {
        return false;
    }

    char* base = (char*)arr->data;
    void* dest = base + arr->size * arr->element_size;
    memcpy(dest, element, arr->element_size);
    arr->size += 1;
    return true;
}

void* array_get(Array* arr, size_t index)
{
    if (index >= arr->size) {
        printf("Array index out of bounds\n");
        exit(EXIT_FAILURE);
    }
    char* base = (char*)arr->data;
    return base + index * arr->element_size;
}


