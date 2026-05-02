#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <raylib.h>
#include <raymath.h>

typedef struct {
    uint32_t id; // uint64_t id;
    uint32_t index;
    // NOTE(holycenjoyer): if you're creating/destroying a lot of entities, you might
    // want to make the id a uint64_t in case of overflow! could also generate
    // a unique UUID per entity id.
} EntityHandle;

typedef enum {
    EN_nil,
    EN_player,
    EN_enemy,
    // :kind
} EntityKind;

typedef struct {
    EntityHandle handle;
    EntityKind kind;

    // transform
    Vector2 pos, size;

    // sprite
    Color color;

    // physics
    Vector2 vel;
    Rectangle collider;
    float speed;
    // :entity
} Entity;

#define MAX_ENTITIES 1024

typedef struct {
    Entity entities[MAX_ENTITIES];
    uint32_t entity_count, last_entity_id;
    Entity *player;
    // :world
} World;

static World *world;
static Entity *nil_en; // thanks to @RuslanKovtun for the suggestion!!!
static float dt;

static inline Entity *entity_from_handle(EntityHandle handle) {
    if (handle.index == 0 || handle.index > world->entity_count)
        return nil_en;
    Entity *en = &world->entities[handle.index];
    if (en->handle.id != handle.id)
        return nil_en;
    return en;
}

static inline int is_entity_valid(EntityHandle handle) {
    if (handle.index == 0 || handle.index > world->entity_count)
        return 0;
    Entity *en = &world->entities[handle.index];
    if (en->handle.id != handle.id)
        return 0;
    return 1;
}

static inline Vector2 get_input_axis(void) {
    const float h = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);
    const float v = IsKeyDown(KEY_S) - IsKeyDown(KEY_W);
    const Vector2 input_axis = (Vector2){ h, v };
    if (Vector2Equals(input_axis, Vector2Zero()))
        return Vector2Zero();
    return Vector2Normalize(input_axis);
}

Entity *entity_create(EntityKind kind) {
    if (world->entity_count >= MAX_ENTITIES) {
        fprintf(stderr, "ran out of entities\n");
        exit(1);
    }

    if (world->entity_count == 0) world->entity_count++;

    Entity *en = &world->entities[world->entity_count];
    en->handle.index = world->entity_count++;
    en->handle.id = world->last_entity_id++;
    en->kind = kind;

    // :default
    en->color = RED;

    return en;
}

void entity_destroy(EntityHandle handle) {
    Entity *en = entity_from_handle(handle);
    if (en == NULL) {
        fprintf(stderr, "warning: attempt to delete invalid entity\n");
        exit(1);
    }

    if (handle.index == world->entity_count - 1) {
        memset(en, 0, sizeof(*en));
        world->entity_count--;
        return;
    }

    Entity *last_en = &world->entities[world->entity_count];
    last_en->handle.index = handle.index; // thanks to @Gargoylee83
    memmove(en, last_en, sizeof(*en));
    world->entity_count--; // thanks to @smx75
}

// :player
void player_setup(Entity *en) {
    world->player = en;
    en->size = (Vector2){ 50.0f, 50.0f };
    en->speed = 500.0f;
    en->color = GREEN;

    en->collider.width = en->size.x, en->collider.height = en->size.y;
}

void player_update(Entity *en) {
    en->vel = Vector2Lerp(
            en->vel,
            Vector2Scale(get_input_axis(), en->speed),
            20.0f * dt
            );
}

// :enemy
void enemy_setup(Entity *en) {
    en->size = (Vector2){ 50.0f, 50.0f };
    en->speed = 100.0f;
    en->vel.x = en->speed;

    en->collider.width = en->size.x, en->collider.height = en->size.y;
}

void enemy_update(Entity *en) {
    if (CheckCollisionRecs(world->player->collider, en->collider))
        entity_destroy(en->handle);
}

void world_update(void) {
    for (int i = 0; i <= world->entity_count; i++) {
        Entity *en = &world->entities[i];

        en->pos = Vector2Add(en->pos, Vector2Scale(en->vel, dt));
        en->collider.x = en->pos.x, en->collider.y = en->pos.y;

        switch (en->kind) {
            case EN_player:
                player_update(en);
                break;

            case EN_enemy:
                enemy_update(en);
                break;

            default: break;
        }
    }
}

void world_draw(void) {
    for (int i = 0; i <= world->entity_count; i++) {
        Entity *en = &world->entities[i];
        DrawRectangleV(en->pos, en->size, en->color);
    }
}

int main(void) {
    InitWindow(800, 600, "test");

    world = calloc(1, sizeof(*world));
    nil_en = &world->entities[0];

    Entity *player = entity_create(EN_player);
    player_setup(player);

    for (int i = 0; i <= 10; i++) {
        Entity *enemy = entity_create(EN_enemy);
        enemy_setup(enemy);
        enemy->pos.x += 100.0f;
        enemy->pos.y += enemy->size.y * (float)i;
    }

    while (!WindowShouldClose()) {
        dt = GetFrameTime();
        world_update();

        BeginDrawing();
        ClearBackground(BLUE);

        world_draw();

        DrawFPS(1, 1);
        EndDrawing();
    }

    free(world);

    CloseWindow();
    return 0;
}
