#include "stdbool.h"
#include <stdint.h>
#include <raylib.h>
#include <stdint.h>

#define CHUNK_WIDTH 100
#define CHUNK_HEIGHT 100

typedef uint64_t BlockType;
#define BLOCK_EMPTY 0
#define BLOCK_WATER 1
#define BLOCK_SAND 2
#define BLOCK_GRAVEL 3

typedef struct {
  Vector2 pos;
  Vector2 last_pos;
} Player;

typedef struct {
  int x;
  int y;
} BlockPos;

static Player player = {
  .pos = (Vector2){0, 0},
  .last_pos = (Vector2){0, 0},
};

bool IsBlockSolid(BlockPos pos) {
  if (pos.x > 0) return true;
  return false;
}

BlockPos GetBlockPos(Vector2 pos) {
  return (BlockPos){
    .x = (int)pos.x,
    .y = (int)pos.y,
  };
}

bool IsPosSolid(Vector2 pos) {
  return IsBlockSolid(GetBlockPos(pos));
}

typedef struct {
  float x;
  float y;
  float w;
  float h;
} Hitbox;

bool IsHitboxColliding(Hitbox a, Hitbox b) {
  if (a.x <= b.x + b.w && a.x + a.w >= b.x && a.y <= b.y + b.h && a.y + a.h >= b.y) return true;
  return false;
}

typedef uint8_t SideMask;

#define SIDE_TOP 1
#define SIDE_RIGHT 2
#define SIDE_BOTTOM 4
#define SIDE_LEFT 8

/* gets the side/s of hitbox a where it collides with hitbox b */
SideMask GetHitboxCollision(Hitbox a, Hitbox b) {
  SideMask res = 0;
  if (IsHitboxColliding(a, b)) return res;
  if (a.y + a.h/2 >= b.y - b.h/2 && a.y + a.h/2 <= b.y + b.h/2) res |= SIDE_TOP;
  if (a.x + a.w/2 >= b.x - b.w/2 && a.x + a.w/2 <= b.x + b.w/2) res |= SIDE_RIGHT;
  if (a.y - a.h/2 >= b.y - b.h/2 && a.y - a.h/2 <= b.y + b.h/2) res |= SIDE_BOTTOM;
  if (a.x - a.w/2 >= b.x - b.w/2 && a.x - a.w/2 <= b.x + b.w/2) res |= SIDE_LEFT;
  return res;
}

float GetHitboxOverlap(Hitbox a, Hitbox b, uint8_t side) {
  if (side == SIDE_TOP) return a.y + a.h/2 - (b.y - b.y/2);
  if (side == SIDE_RIGHT) return a.x + a.w/2 - (b.x - b.x/2);
  if (side == SIDE_BOTTOM) return b.y + b.h/2 - (a.y - a.y/2);
  if (side == SIDE_LEFT) return b.x + b.w/2 - (a.x - a.x/2);
  return 0;
}
