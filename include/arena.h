#ifndef ARENA_H
#define ARENA_H

#include "utils.h"
#define ALIGNMENT (2 * sizeof(void *))
#define is_power_of_two(x) ((x != 0) && ((x & (x - 1)) == 0))

typedef struct {
	void* base;
	size_t capacity;
	size_t size;
	void *cur;
} Arena;

uintptr_t align_forward(uintptr_t ptr, size_t alignment);

void* arena_alloc(Arena* arena, size_t size);

void arena_free(Arena* arena);

Arena arena_init(size_t capacity);

#endif