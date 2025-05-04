#include "stdbool.h"
#include <raylib.h>

typedef struct {
  Vector2 pos;
  Vector2 last_pos;
} Player;

typedef struct {
  int x;
  int y;
} BlockPos;

bool IsBlockSolid(BlockPos pos) {
  if (pos.x > 0) return true;
  return false;
}
