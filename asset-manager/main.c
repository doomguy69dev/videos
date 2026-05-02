#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <dirent.h>
#include <raylib.h>

#define STB_DS_IMPLEMENTATION
#include <stb/stb_ds.h>

typedef struct {
    char *key;
    Texture2D value;
} Sprite_HashEntry;

static struct {
    Sprite_HashEntry *sprites;
} assets = {0};

void load_sprites(char *const res_path) {
    hmdefault(assets.sprites, LoadTexture("res/sprites/ded.png"));

    struct dirent *dp;
    DIR *dir = opendir(res_path);
    assert(dir);

    char path[128];

    uint32_t i = 0;
    while (dp = readdir(dir)) {
        if (i > 1) {
            snprintf(path, sizeof(path), "%s/%s", res_path, dp->d_name);
            char name[128];
            strncpy(name, dp->d_name, sizeof(name));
            name[sizeof(name) - 1] = '\0';
            char *sprite_name = strtok(name, ".");
            shput(assets.sprites, strdup(sprite_name), LoadTexture(path));
        }
        i++;
    }
}

static inline Texture2D get_sprite(char *const name) {
    return shget(assets.sprites, name);
}

int main(void) {
    InitWindow(1280, 720, "test");

    load_sprites("res/sprites");

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground((Color){ 20, 20, 20, 255 });

        DrawTexture(get_sprite("sdksnjdskdjsdj"), 0, 0, WHITE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
