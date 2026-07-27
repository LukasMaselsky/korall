#include "array.h"


Array* array_create_heap(const size_t element_size, const size_t capacity) {

	Array* arr = (Array*)malloc(sizeof(Array));

	if (arr == NULL) return arr;

	arr->size = 0;
	arr->capacity = capacity;
	arr->element_size = element_size;
	arr->on_heap = true;
	arr->data = malloc(capacity * element_size);
	if (arr->data == NULL) {
		free(arr);
		arr = NULL;
	}
	return arr;
}

void array_free(Array* arr) {
	if (arr == NULL) return;
	if (!arr->on_heap) return;

	free(arr->data);
	free(arr);
	return;
}

void* array_get(const Array* arr, const size_t index) {
	if (arr == NULL) return NULL;
	if (index >= arr->size) return NULL;
	return arr->data + (index * arr->element_size);
}

int array_push(Array* arr, const void* item) {
	if (arr == NULL) return -1;
	if (arr->size >= arr->capacity) return -1;
	
	memcpy(arr->data + (arr->size * arr->element_size), item, arr->element_size);
	arr->size++;

	return 0;
}

void array_pop(Array* arr, void* item) {
	if (arr == NULL) return;
	if (arr->size == 0) return;

	uint8_t* last = arr->data + ((arr->size - 1) * arr->element_size);
	if (item != NULL) {
		memcpy(item, last, arr->element_size);
	}
	memset(last, 0, arr->element_size);
	arr->size--;
	return;
}

int array_remove(Array* arr, const size_t index) {
	if (arr == NULL) return -1;
	if (index >= arr->size) return -1;

	if (index == arr->size - 1) {
		array_pop(arr, NULL);
		return 0;
	}
	uint8_t* right = arr->data + ((index + 1) * arr->element_size);
	uint8_t* left = arr->data + (index * arr->element_size);
	memmove(left, right, (arr->size - index - 1) * arr->element_size);
	array_pop(arr, NULL);
	return 0;
}

/**
 * @brief removes list of items
 * @param arr 
 * @param indices must be in ascending order
 * @param len 
 * @return 
 */
int array_remove_list(Array* arr, const size_t* indices, const size_t len) {
	if (arr == NULL) return -1;
	if (len == 0) return -1;
	size_t i = len - 1;
	while (true) {
		size_t index = indices[i];
		array_remove(arr, index);
		if (i == 0) break;
		i--;
	}

	return 0;
}

int array_set(Array* arr, const size_t index, const void* item) {
	if (arr == NULL) return -1;
	if (index >= arr->size) return -1;

	memcpy(arr->data + (index * arr->element_size), item, arr->element_size);

	return 0;
}

int array_find(Array* arr, void* item, bool (* const compare)(const void *, const void*)) {
	if (arr == NULL) return -1;
	for (size_t i = 0; i < arr->size; i++) {
		void* element = array_get(arr, i);
		if (compare(item, element)) return i;
	}
	return -1;
}

void array_clear(Array* arr) {
	if (arr == NULL) return;
	if (arr->size == 0) return;

	memset(arr->data, 0, arr->size * arr->element_size);
	arr->size = 0;
	return;
}

bool array_is_empty(Array* arr) {
	if (arr == NULL) return false;
	return arr->size == 0;
}

bool array_full(Array* arr) {
	if (arr == NULL) return false;
	return arr->size >= arr->capacity;
}

void array_for_each(Array* arr, void (* const callback)(const void*, va_list), ...) {
	if (arr == NULL) return -1;
	for (size_t i = 0; i < arr->size; i++) {
		void* element = array_get(arr, i);
		va_list argp;
		va_start(argp, callback);
		callback(element, argp);
		va_end(argp);
	}
}