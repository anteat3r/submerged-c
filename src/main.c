#include "raylib.h"
#include <emscripten/emscripten.h>

#include "tilemap.c"

int main() {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(300, 300, "Submerged");
  emscripten_set_main_loop(tick, 0, 1);
  CloseWindow();
}
