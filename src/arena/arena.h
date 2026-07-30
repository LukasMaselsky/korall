#ifndef KORALL_ARENA_H
#define KORALL_ARENA_H

#include <stdlib.h>
#include "utils/utils.h"
#define ALIGNMENT (2 * sizeof(void *))
#define is_power_of_two(x) ((x != 0) && ((x & (x - 1)) == 0))

typedef struct {
	void* base;
	size_t capacity;
	size_t size;
	void *cur;
} Arena;

uintptr_t align_forward(uintptr_t ptr, size_t alignment);

uintptr_t arena_alloc(Arena* arena, size_t size);

void arena_clear(Arena* arena);

void arena_free(Arena* arena);

Arena arena_init(size_t capacity);

#endif