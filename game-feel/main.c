#include <raylib.h>
#include <raymath.h>

#define v2(x,y) (Vector2){ (x), (y) }
static float dt;
static const float rots[] = { 10.0f, -10.0f };

static inline Vector2 get_input_axis(void) {
    const Vector2 input_axis = (Vector2){
        IsKeyDown(KEY_D) - IsKeyDown(KEY_A),
        IsKeyDown(KEY_S) - IsKeyDown(KEY_W),
    };
    if (Vector2Equals(input_axis, Vector2Zero()))
        return Vector2Zero();
    return Vector2Normalize(input_axis);
}

typedef struct {
    Vector2 pos, render_pos, size, scale, origin;
    float rot;
    Vector2 vel;
    float speed;
} Player;

Player player_create(void) {
    Player p = {0};
    p.pos = v2(20.0f, 20.0f), p.size = v2(40.0f, 60.0f);
    p.scale = Vector2One();
    p.speed = 300.0f;
    return p;
}

void player_update(Player *p) {
    p->scale = Vector2Lerp(p->scale, Vector2One(), 15.0f * dt);
    p->rot = Lerp(p->rot, 0.0f, 15.0f * dt);
    p->pos = Vector2Add(p->pos, Vector2Scale(p->vel, dt));

    p->vel = Vector2Lerp(p->vel, Vector2Scale(get_input_axis(), p->speed), 18.0f * dt);

    if (IsKeyPressed(KEY_SPACE)) {
        p->scale = v2(0.8f, 0.8f);
        p->rot = rots[GetRandomValue(0, 1)];
        p->vel.x -= (float)GetRandomValue(200.0f, 400.0f);
    }
}

void player_draw(Player *p) {
    p->render_pos = p->pos;

    // this is shit, but it works kek
    const Vector2 input_axis = get_input_axis();
    if (!Vector2Equals(input_axis, Vector2Zero())) {
        const float bop = fabsf(sinf(GetTime() * 6.5f) * 20.0f);
        p->render_pos.y -= bop;
        p->rot = sinf(GetTime() * 6.0f) * 3.0f;
        // forgot to add this in the video OwO
        if (bop < 0.8f)
            p->scale.y = 0.8f;
    }

    const Rectangle rect = (Rectangle){
        p->render_pos.x, p->render_pos.y,
        p->size.x * p->scale.x, p->size.y * p->scale.y,
    };
    const Vector2 origin = Vector2Scale(v2(rect.width, rect.height), 0.5f);
    DrawRectanglePro(rect, origin, p->rot, RED);
}

int main(void) {
    InitWindow(800, 600, "test");

    Player player = player_create();

    while (!WindowShouldClose()) {
        dt = GetFrameTime();
        player_update(&player);

        BeginDrawing();
        ClearBackground(BLUE);

        player_draw(&player);

        DrawFPS(1, 1);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

