#if defined HEADERS
#include "array/array.h"
#elif defined TESTS

TEST("array_create_stack") {
	Array arr = { 0 };
	int data[30] = { 0 };
	array_create_stack(&arr, data, sizeof(int), 30);
	ASSERT(arr.capacity == 30);
	ASSERT(arr.size == 0);
	ASSERT(arr.element_size == sizeof(int));
	ASSERT(arr.on_heap == false);
	ASSERT(arr.data == data);
}

TEST("array_create_heap") {
	Array* arr = array_create_heap(sizeof(int), 30);
	ASSERT(arr->capacity == 30);
	ASSERT(arr->size == 0);
	ASSERT(arr->element_size == sizeof(int));
	ASSERT(arr->on_heap == true);
	
}

TEST("array_get") {
	Array* arr = array_create_heap(sizeof(int), 30);
	int num = 0;
	array_push(arr, &num);
	num = 1;
	array_push(arr, &num);
	num = 2;
	array_push(arr, &num);
	num = 3;
	array_push(arr, &num);
	num = 4;
	array_push(arr, &num);
	num = 5;

	ASSERT(*((int*)array_get(arr, 0)) == 0);
	ASSERT(*((int*)array_get(arr, 1)) == 1);
	ASSERT(*((int*)array_get(arr, 2)) == 2);
	ASSERT(*((int*)array_get(arr, 3)) == 3);
	ASSERT(*((int*)array_get(arr, 4)) == 4);
	ASSERT(array_get(arr, 5) == NULL);
	array_pop(arr, NULL);
	ASSERT(array_get(arr, 4) == NULL);
	array_remove(arr, 1);
	ASSERT(*((int*)array_get(arr, 1)) == 2);
	
	array_clear(arr);
	
	ASSERT(array_get(arr, 0) == NULL);
	ASSERT(array_get(arr, 1) == NULL);
	ASSERT(array_get(arr, 2) == NULL);
}


TEST("array_find") {

}



#endif