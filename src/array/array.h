#ifndef ARRAY_H
#define ARRAY_H

#include "utils/utils.h"


typedef struct {
	uint8_t* data;
	size_t element_size;
	size_t size;
	size_t capacity;
	bool on_heap;
} Array;

#define array_create_stack(a_data, a_element_size, a_capacity) { .data = a_data, .size = 0, .capacity = a_capacity, .element_size = a_element_size, .on_heap = false };

Array* array_create_heap(const size_t element_size, const size_t capacity);

void array_free(Array* arr);

void* array_get(const Array* arr, const size_t index);

int array_push(Array* arr, const void* item);

void array_pop(Array* arr, void* item);

int array_remove(Array* arr, const size_t index);

int array_remove_list(Array* arr, const size_t* indices, const size_t len);

int array_set(Array* arr, const size_t index, const void* item);

int array_find(Array* arr, void* item, bool (* const compare)(const void*, const void*));

void array_clear(Array* arr);

bool array_empty(Array* arr);

bool array_full(Array* arr);

void array_for_each(Array* arr, void (* const callback)(const void*, va_list), ...);

#endif