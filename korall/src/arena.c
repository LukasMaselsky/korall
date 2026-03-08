#include "arena.h"
#include "utils.h"

uintptr_t align_forward(uintptr_t ptr, size_t alignment) {
    uintptr_t p, a, modulo;
    if (!is_power_of_two(alignment)) {
        return 0;
    }

    p = ptr;
    a = (uintptr_t)alignment;
    modulo = p & (a - 1);

    if (modulo) {
        p += a - modulo;
    }

    return p;
}

Arena arena_init(size_t capacity) {
    void* buf = safe_calloc(1, capacity);
    Arena arena = { .base = buf, .capacity = capacity, .cur = buf, .size = 0 };
    return arena;
}

void arena_clear(Arena* arena) {
    memset(arena->base, 0, arena->capacity);
    arena->size = 0;
    arena->cur = arena->base;
}

void arena_free(Arena* arena) {
    free(arena->base);
    arena->base = NULL;
    arena->capacity = 0;
    arena->size = 0;
    arena->cur = NULL;
}

void* arena_alloc(Arena *arena, size_t size) {
    if (size == 0) return 0;
   
    size_t alignment = ALIGNMENT;
    uintptr_t cur = (uintptr_t)arena->cur;
    uintptr_t new_cur = align_forward(cur, alignment);
    unsigned int ali_size = new_cur - (uintptr_t)arena->base;

    if (ali_size + size > arena->capacity) {
        fprintf(stderr, "Fatal: failed to allocate %zu bytes in arena, over capacity.\n", size);
        exit(EXIT_FAILURE);
    }

    arena->size = ali_size + size;
    arena->cur = new_cur + size;

    return new_cur;
}
