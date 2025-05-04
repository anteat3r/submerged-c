#include "raylib.h"
#include <stdio.h>

void UpdateDrawFrame() {
  BeginDrawing();
  ClearBackground(BLACK);
  DrawCircle(100, 100, 10, RED);
  char str[10];
  sprintf(str, "%d %d", GetScreenWidth(), GetScreenHeight());
  DrawText(str, 30, 30, 30, WHITE);
  EndDrawing();
}
