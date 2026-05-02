static const char *vs_src = "#version 330 core\n"
"layout (location = 0) in vec2 a_pos;"
"layout (location = 1) in vec4 a_color;"
"layout (location = 2) in vec2 a_uv;"
"layout (location = 3) in float a_texid;"
"out vec4 color;"
"out vec2 uv;"
"out float texid;"
"uniform mat4 u_proj;"
"void main() {"
    "gl_Position = u_proj * vec4(a_pos, 0.0, 1.0);"
    "color = a_color;"
    "uv = a_uv;"
    "texid = a_texid;"
"}";
static const char *fs_src = "#version 330 core\n"
"in vec4 color;"
"in vec2 uv;"
"in float texid;"
"out vec4 out_color;"
"uniform sampler2D u_smp;"
"void main() {"
    "if (texid == 0.0) {"
        "out_color = color;"
    "} if (texid == 1.0) {"
        "out_color = texture(u_smp, uv) * color;"
    "}"
"}";

#define GFX_MAX_QUADS 4096
#define GFX_MAX_VERTICES GFX_MAX_QUADS * 4
#define GFX_SPRITE_ATLAS_SIZE 2048

typedef struct {
    vec2s pos;
    vec4s color;
    vec2s uv;
    float texid;
    // texid: 0 = rect
    //        1 = sprite
    //        2 = font
} GfxVertex;
typedef GfxVertex GfxQuad[4];

typedef struct {
    vec2s pos, size;
    vec4s color;
} GfxRect;

typedef struct {
    mat4s proj;
    vec2s pos;
    float rot, zoom;
} GfxCamera;

typedef struct {
    uint8_t *pixels;
    uint32_t w, h;
} GfxImage;

typedef struct {
    vec4s uv;
    uint32_t w, h;
} GfxSprite;

typedef struct {
    GfxQuad quads[GFX_MAX_QUADS];
    uint32_t quad_count;
    struct {
        uint32_t vao, vbo, ibo;
        uint32_t shd, u_proj_loc;
    } gl;
} GfxRenderData;
static GfxRenderData *rd;

void gfx_init(void) {
    rd = calloc(1, sizeof(*rd));
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    uint32_t vs, fs;
    int result; char info_log[1024];
    vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vs_src, NULL);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &result);
    if (!result) {
        glGetShaderInfoLog(vs, 1024, NULL, info_log);
        fprintf(stderr, "vertex shader compilation failed, error: %s\n", info_log);
        exit(1);
    }
    fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fs_src, NULL);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &result);
    if (!result) {
        glGetShaderInfoLog(fs, 1024, NULL, info_log);
        fprintf(stderr, "fragment shader compilation failed, error: %s\n", info_log);
        exit(1);
    }
    rd->gl.shd = glCreateProgram();
    glAttachShader(rd->gl.shd, vs);
    glAttachShader(rd->gl.shd, fs);
    glLinkProgram(rd->gl.shd);
    glGetProgramiv(rd->gl.shd, GL_LINK_STATUS, &result);
    if (!result) {
        glGetProgramInfoLog(rd->gl.shd, 1024, NULL, info_log);
        fprintf(stderr, "shader program linking failed, error: %s\n", info_log);
        exit(1);
    }
    glUseProgram(rd->gl.shd);
    rd->gl.u_proj_loc = glGetUniformLocation(rd->gl.shd, "u_proj");

    glGenVertexArrays(1, &rd->gl.vao);
    glBindVertexArray(rd->gl.vao);

    glGenBuffers(2, &rd->gl.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, rd->gl.vbo);
    glBufferData(GL_ARRAY_BUFFER, GFX_MAX_VERTICES * sizeof(GfxVertex), NULL, GL_DYNAMIC_DRAW);

    uint32_t indices[GFX_MAX_QUADS * 6];
    for (uint32_t i = 0; i < GFX_MAX_QUADS; i++) {
        indices[i * 6] = i * 4;
        indices[i * 6 + 1] = i * 4 + 1;
        indices[i * 6 + 2] = i * 4 + 3;
        indices[i * 6 + 3] = i * 4 + 1;
        indices[i * 6 + 4] = i * 4 + 2;
        indices[i * 6 + 5] = i * 4 + 3;
    }
    glGenBuffers(1, &rd->gl.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rd->gl.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GfxVertex), (void*)offsetof(GfxVertex, pos));
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(GfxVertex), (void*)offsetof(GfxVertex, color));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GfxVertex), (void*)offsetof(GfxVertex, uv));
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(GfxVertex), (void*)offsetof(GfxVertex, texid));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
}

void gfx_uninit(void) {
    free(rd);
}

void gfx_begin(GfxCamera camera) {
    glClear(GL_COLOR_BUFFER_BIT);
    rd->quad_count = 0;

    camera.proj = glms_ortho(0.0f, (float)win_w, (float)win_h, 0.0f, -1.0f, 1.0f);
    camera.proj = glms_rotate_z(camera.proj, camera.rot);
    camera.proj = glms_translate(camera.proj, (vec3s){{ -camera.pos.x, -camera.pos.y, 0.0f }});
    camera.proj = glms_scale(camera.proj, (vec3s){{ camera.zoom, camera.zoom, 1.0f }});
    glUniformMatrix4fv(rd->gl.u_proj_loc, 1, GL_FALSE, &camera.proj.m00);
}

void gfx_end(void) {
    glBufferSubData(GL_ARRAY_BUFFER, 0, rd->quad_count * 4 * sizeof(GfxVertex), rd->quads);
    glDrawElements(GL_TRIANGLES, rd->quad_count * 6, GL_UNSIGNED_INT, NULL);
}

void gfx_push_rect(GfxRect rect) {
    if (rd->quad_count >= GFX_MAX_QUADS) {
        fprintf(stderr, "ran out of quads\n");
        exit(1);
    }

    GfxVertex *quad = (GfxVertex*)&rd->quads[rd->quad_count];
    quad[0] = (GfxVertex){ rect.pos, rect.color, GLMS_VEC2_ZERO, 0.0f };
    quad[1] = (GfxVertex){ {{ rect.pos.x + rect.size.x, rect.pos.y }}, rect.color, GLMS_VEC2_ZERO, 0.0f };
    quad[2] = (GfxVertex){ {{ rect.pos.x + rect.size.x, rect.pos.y + rect.size.y }}, rect.color, GLMS_VEC2_ZERO, 0.0f };
    quad[3] = (GfxVertex){ {{ rect.pos.x, rect.pos.y + rect.size.y }}, rect.color, GLMS_VEC2_ZERO, 0.0f };
    rd->quad_count++;
}

void gfx_push_sprite(GfxRect rect, GfxSprite sprite) {
    if (rd->quad_count >= GFX_MAX_QUADS) {
        fprintf(stderr, "ran out of quads\n");
        exit(1);
    }

    GfxVertex *quad = (GfxVertex*)&rd->quads[rd->quad_count];
    quad[0] = (GfxVertex){ rect.pos, rect.color, (vec2s){{ sprite.uv.x, sprite.uv.y }}, 1.0f };
    quad[1] = (GfxVertex){ {{ rect.pos.x + rect.size.x, rect.pos.y }}, rect.color, (vec2s){{ sprite.uv.x + sprite.uv.z, sprite.uv.y }}, 1.0f };
    quad[2] = (GfxVertex){ {{ rect.pos.x + rect.size.x, rect.pos.y + rect.size.y }}, rect.color, (vec2s){{ sprite.uv.x + sprite.uv.z, sprite.uv.y + sprite.uv.w }}, 1.0f };
    quad[3] = (GfxVertex){ {{ rect.pos.x, rect.pos.y + rect.size.y }}, rect.color, (vec2s){{ sprite.uv.x, sprite.uv.y + sprite.uv.w }}, 1.0f };
    rd->quad_count++;
}

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb/stb_image.h"

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb/stb_rect_pack.h"

GfxImage gfx_image_load(Arena *arena, const char *path) {
    GfxImage img;

    int w, h, c;
    uint8_t *pixels = stbi_load(path, &w, &h, &c, 4);
    c = 4;
    if (!pixels) {
        fprintf(stderr, "failed to load %s\n", path);
        exit(1);
    }
    img.w = w, img.h = h;

    img.pixels = arena_alloc(arena, w * h * c);
    memcpy(img.pixels, pixels, w * h * c);

    stbi_image_free(pixels);

    return img;
}

void gfx_sprite_atlas_init(GfxImage *images, GfxSprite *sprites, uint32_t image_count) {
    const size_t node_count = GFX_SPRITE_ATLAS_SIZE;
    const size_t atlas_size = GFX_SPRITE_ATLAS_SIZE * GFX_SPRITE_ATLAS_SIZE * 4;

    Arena atlas_arena = arena_create(
        sizeof(stbrp_node) * node_count +
        sizeof(stbrp_rect) * image_count +
        atlas_size
    );

    stbrp_context ctx;
    stbrp_node *nodes = arena_alloc(&atlas_arena, sizeof(*nodes) * node_count);
    stbrp_rect *rects = arena_alloc(&atlas_arena, sizeof(*rects) * image_count);
    uint8_t *pixels = arena_alloc(&atlas_arena, atlas_size);

    stbrp_init_target(&ctx, GFX_SPRITE_ATLAS_SIZE, GFX_SPRITE_ATLAS_SIZE, nodes, node_count);
    for (uint32_t i = 0; i < image_count; i++) {
        stbrp_rect *r = &rects[i];
        GfxImage *img = &images[i];
        r->w = img->w, r->h = img->h;
        r->id = i;
    }

    if (!stbrp_pack_rects(&ctx, rects, image_count)) {
        fprintf(stderr, "sprite atlas size not enough\n");
        exit(1);
    }

    for (uint32_t i = 0; i < image_count; i++) {
        const stbrp_rect r = rects[i];
        GfxImage *img = &images[i];
        GfxSprite *sprite = &sprites[i];

        for (uint32_t y = 0; y < r.h; y++) {
            uint8_t *dst_row = pixels + ((r.y + y) * GFX_SPRITE_ATLAS_SIZE + r.x) * 4;
            uint8_t *src_row = img->pixels + (y * img->w) * 4;
            memcpy(dst_row, src_row, r.w * 4);
        }

        sprite->uv.x = (float)r.x / (float)GFX_SPRITE_ATLAS_SIZE;
        sprite->uv.y = (float)r.y / (float)GFX_SPRITE_ATLAS_SIZE;
        sprite->uv.z = (float)r.w / (float)GFX_SPRITE_ATLAS_SIZE;
        sprite->uv.w = (float)r.h / (float)GFX_SPRITE_ATLAS_SIZE;
        sprite->w = img->w, sprite->h = img->h;
    }

    uint32_t atlas_texture;
    glGenTextures(1, &atlas_texture);
    glBindTexture(GL_TEXTURE_2D, atlas_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GFX_SPRITE_ATLAS_SIZE, GFX_SPRITE_ATLAS_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    arena_destroy(&atlas_arena);
}
