#include "raylib.h"
#include "globals.c"
#include <stdint.h>

static Camera2D camera = {
  (Vector2){0, 0},
  (Vector2){0, 0},
  0, 1,
};

static uint64_t frame_count = 0;

void UpdateDrawFrame() {
  frame_count++;
  if (frame_count == UINT64_MAX)
    frame_count = 0;

  if (frame_count % 10 == 0) {
    camera.offset.x = (float)GetScreenWidth()/2;
    camera.offset.y = (float)GetScreenHeight()/2;
  }

  BeginDrawing();
  ClearBackground(BLACK);
  BeginMode2D(camera);
  EndMode2D();
  EndDrawing();
}
