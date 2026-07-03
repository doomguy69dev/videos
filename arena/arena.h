#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>

typedef struct {
    void *data;
    size_t size, offset;
} Arena;

Arena arena_create(size_t size);
void arena_destroy(Arena *arena);
void *arena_alloc(Arena *arena, size_t size);
void arena_reset(Arena *arena);

#endif // ARENA_H

#ifdef ARENA_IMPL

#include <assert.h>
#include <stdlib.h>

Arena arena_create(size_t size) {
    return (Arena){
        .size = size,
        .data = calloc(1, size),
    };
}

void arena_destroy(Arena *arena) {
    free(arena->data);
    arena->size = 0, arena->offset = 0;
}

void *arena_alloc(Arena *arena, size_t size) {
    assert(size <= arena->size - arena->offset);

    void *ptr = (void*)((size_t)arena->data + arena->offset);
    arena->offset += size;

    return ptr;
}

void arena_reset(Arena *arena) {
    arena->offset = 0;
}

#endif // ARENA_IMPL
