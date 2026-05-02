#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define CGLM_USE_ANONYMOUS_STRUCT 1
#include <cglm/struct.h>

#define v2(x,y) (vec2s){{(x), (y)}}
#define v4(x,y,z,w) (vec4s){{(x), (y), (z), (w)}}

static uint32_t win_w = 1280, win_h = 720;

#define ARENA_IMPL
#include "arena.h"
#include "gfx.c"

static void framebuffer_resize_callback(GLFWwindow *win, int w, int h) {
    glViewport(0, 0, w, h);
    win_w = w, win_h = h;
}

#define SPRITES\
    X(SPRITE_bebe, "bebe.png")\
    X(SPRITE_dude, "dude.png")\
    X(SPRITE_ded, "ded.png")

enum {
#define X(id, path) id,
    SPRITES
#undef X
    MAX_SPRITES
};

int main(void) {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow *win = glfwCreateWindow(win_w, win_h, "test", NULL, NULL);
    glfwSetFramebufferSizeCallback(win, framebuffer_resize_callback);
    glfwMakeContextCurrent(win);

    glewInit();

    gfx_init();

    Arena image_arena = arena_create(GFX_SPRITE_ATLAS_SIZE * GFX_SPRITE_ATLAS_SIZE * 4);
    GfxImage images[MAX_SPRITES] = {
#define X(id, path) gfx_image_load(&image_arena, "res/"path),
        SPRITES
#undef X
    };
    GfxSprite sprites[MAX_SPRITES];
    gfx_sprite_atlas_init(images, sprites, MAX_SPRITES);
    arena_destroy(&image_arena);

    GfxCamera camera = (GfxCamera){
        .zoom = 1.0f,
    };

    vec2s pos = {0};

    float current_time, last_time, delta_time;
    while (!glfwWindowShouldClose(win)) {
        current_time = (float)glfwGetTime();
        delta_time = current_time - last_time;
        last_time = current_time;
        glfwPollEvents();

        if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS) pos.x += 50.0f * delta_time;
        if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS) pos.x -= 50.0f * delta_time;
        if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS) pos.y -= 50.0f * delta_time;
        if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS) pos.y += 50.0f * delta_time;
        if (glfwGetKey(win, GLFW_KEY_SPACE) == GLFW_PRESS) camera.pos.y += 50.0f * delta_time;

        camera.zoom = (float)win_h / 180.0f;

        gfx_begin(camera);
        gfx_push_sprite((GfxRect){ .pos = pos, .size = v2(32.0f, 32.0f), .color = GLMS_VEC4_ONE }, sprites[SPRITE_ded]);
        gfx_push_sprite((GfxRect){ .pos = v2(10.0f, 10.0f), .size = v2(32.0f, 32.0f), .color = GLMS_VEC4_ONE }, sprites[SPRITE_dude]);
        gfx_push_sprite((GfxRect){ .pos = v2(100.0f, 100.0f), .size = v2(32.0f, 32.0f), .color = GLMS_VEC4_ONE }, sprites[SPRITE_bebe]);
        gfx_end();

        glfwSwapBuffers(win);
    }

    gfx_uninit();

    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}
