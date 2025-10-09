#include "raylib.h"

#include <stdint.h>
#include <stdio.h>

// NOTE: See `https://leanrada.com/notes/dynamic-patrol-stealth-games/`.

typedef uint8_t  u8;
typedef uint32_t u32;
typedef int32_t  i32;
typedef float    f32;
typedef double   f64;

typedef struct {
    u32 x, y;
} Vector2u;

#define FPS_X 10
#define FPS_Y FPS_X

#define BACKGROUND ((Color){0x20, 0x20, 0x20, 0xFF})

#define SCREEN_X 480
#define SCREEN_Y SCREEN_X

#define ROWS 10
#define COLS ROWS

#define GRID_X (SCREEN_X / ROWS)
#define GRID_Y (SCREEN_Y / COLS)

#define FRAME_SPEED 5

static u8 WEIGHTS[ROWS][COLS] = {0};

#define WALL '#'

static const char WORLD[ROWS][COLS] = {
    "#...#.....",
    "#.#...#.#.",
    "#.###.#.#.",
    "......#...",
    ".##.#.##.#",
    ".##.#..#.#",
    "....##.#.#",
    "#.#..#...#",
    "#.##.###.#",
    "#........#",
};

static f32 lerp(const f32 l, const f32 r, const f32 t) {
    return l + (t * (r - l));
}

static Vector2u update(const Vector2u position) {
    for (u32 i = 0; i < ROWS; ++i) {
        for (u32 j = 0; j < COLS; ++j) {
            if (WORLD[i][j] == WALL) {
                continue;
            }

            WEIGHTS[i][j] = 0xFF == WEIGHTS[i][j] ? 0xFF : WEIGHTS[i][j] + 1;
        }
    }

    WEIGHTS[position.y][position.x] = 0;

    Vector2u start = position;
    Vector2u end = (Vector2u){position.x + 1, position.y + 1};

    if (start.y != 0) {
        start.y -= 1;
    }

    if (start.x != 0) {
        start.x -= 1;
    }

    if (end.y < ROWS) {
        end.y += 1;
    }

    if (end.x < COLS) {
        end.x += 1;
    }

    Vector2u next_position = position;
    u8       watermark = 0;

    for (u32 i = start.y; i < end.y; ++i) {
        if (watermark < WEIGHTS[i][position.x]) {
            next_position = (Vector2u){position.x, i};
            watermark = WEIGHTS[i][position.x];
        }
    }

    for (u32 j = start.x; j < end.x; ++j) {
        if (watermark < WEIGHTS[position.y][j]) {
            next_position = (Vector2u){j, position.y};
            watermark = WEIGHTS[position.y][j];
        }
    }

    return next_position;
}

static void draw(const Vector2u from, const Vector2u to, const f32 t) {
    for (u32 i = 0; i < ROWS; ++i) {
        for (u32 j = 0; j < COLS; ++j) {
            if (WORLD[i][j] == WALL) {
                continue;
            }

            DrawRectangle(GRID_X * (i32)j, GRID_Y * (i32)i, GRID_X, GRID_Y, RAYWHITE);
            DrawRectangle(GRID_X * (i32)j,
                          GRID_Y * (i32)i,
                          GRID_X,
                          GRID_Y,
                          (Color){BLUE.r, BLUE.g, BLUE.b, WEIGHTS[i][j]});
        }
    }

    DrawRectangleV((Vector2){lerp((f32)from.x, (f32)to.x, t) * GRID_X,
                             lerp((f32)from.y, (f32)to.y, t) * GRID_Y},
                   (Vector2){GRID_X, GRID_Y},
                   ORANGE);
}

i32 main(void) {
    SetTraceLogLevel(LOG_WARNING);

    InitWindow(SCREEN_X, SCREEN_Y, "raypatrol");
    SetTargetFPS(60);

    for (u32 i = 0; i < ROWS; ++i) {
        for (u32 j = 0; j < COLS; ++j) {
            if (WORLD[i][j] == WALL) {
                continue;
            }

            WEIGHTS[i][j] = 0xFF;
        }
    }

    Vector2u from = {1, 1};
    Vector2u to = from;

    u32 frames = 0;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BACKGROUND);

        if (frames == 0) {
            from = to;
            to = update(from);
        }

        draw(from, to, (f32)frames / FRAME_SPEED);

        DrawFPS(FPS_X, FPS_Y);
        EndDrawing();

        frames = (frames + 1) % FRAME_SPEED;
    }

    CloseWindow();

    return 0;
}
