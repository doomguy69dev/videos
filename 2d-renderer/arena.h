#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>

#ifndef ARENA_ASSERT
#include <stdio.h>
#include <stdlib.h>
#define ARENA_ASSERT(c) do {\
    if (!(c)) {\
        fprintf(stderr, "assertion failure at %s:%d: %s\n", __FILE__, __LINE__, #c);\
        abort();\
    }\
} while(0);
#endif

typedef struct {
    void *data;
    size_t offset, cap;
} Arena;

Arena arena_create(size_t size);
void arena_destroy(Arena *arena);
void *arena_alloc(Arena *arena, size_t size);
void arena_reset(Arena *arena); // sets arena.offset to 0
void arena_clear(Arena *arena); // arena_reset but memset 0 the data

#endif // ARENA_H

#ifdef ARENA_IMPL

#ifndef ARENA_ASSERT
#include <stdlib.h> // so we dont include stdlib.h twice
#endif
#include <string.h>

Arena arena_create(size_t size) {
    Arena arena;

    arena.data = calloc(1, size);
    ARENA_ASSERT(arena.data);
    arena.cap = size;
    arena.offset = 0;

    return arena;
}

void arena_destroy(Arena *arena) {
    free(arena->data);
    arena->cap = 0, arena->offset = 0;
}

void *arena_alloc(Arena *arena, size_t size) {
    ARENA_ASSERT(size <= arena->cap - arena->offset);

    void *ptr = (void*)((char*)arena->data + arena->offset);
    arena->offset += size;

    return ptr;
}

void arena_reset(Arena *arena) {
    arena->offset = 0;
}

void arena_clear(Arena *arena) {
    memset(arena->data, 0, arena->cap);
    arena->offset = 0;
}

#endif // ARENA_IMPL
