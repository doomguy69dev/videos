#include <stdio.h>
#include <stdint.h>

#define ARENA_IMPL
#include "arena.h"

#define ATLAS_SIZE 8192

int main(void) {
    Arena images_arena = arena_create(ATLAS_SIZE);
    uint8_t *image1 = arena_alloc(&images_arena, 1024);
    uint8_t *image2 = arena_alloc(&images_arena, 1024);
    uint8_t *image3 = arena_alloc(&images_arena, 2048);

    image3[2047] = 69;
    printf("%d\n", image3[2047]);

    // pack into atlas
    // upload to gpu...
    
    arena_destroy(&images_arena);

    return 0;
}
