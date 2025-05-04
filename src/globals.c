#include "stdbool.h"
#include <raylib.h>
#include <cstdint>

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

typedef struct {
  float x;
  float y;
  float w;
  float h;
} Hitbox;

bool IsHitboxColliding(Hitbox a, Hitbox b) {
  if (a.x <= b.x + b.width && a.x + a.width >= b.x && a.y <= b.y + b.height && a.y + a.height >= b.y) return true;
  return false;
}

typedef uint8_t SideMask;

#define TOP 1
#define RIGHT 2
#define BOTTOM 4
#define LEFT 8

/* gets the side/s of hitbox a where it collides with hitbox b */
SideMask GetHitboxCollision(Hitbox a, Hitbox b) {
  SideMask res = 0;
  if IsHitboxColliding(a, b) return res;
  if (a.y + a.h/2 >= b.y - b.h/2 && a.y + a.h/2 <= b.y + b.h/2) res |= TOP;
  if (a.x + a.w/2 >= b.x - b.w/2 && a.x + a.w/2 <= b.x + b.w/2) res |= RIGHT;
  if (a.y - a.h/2 >= b.y - b.h/2 && a.y - a.h/2 <= b.y + b.h/2) res |= BOTTOM;
  if (a.x - a.w/2 >= b.x - b.w/2 && a.x - a.w/2 <= b.x + b.w/2) res |= LEFT;
  return res;
}

float GetHitboxOverlap(Hitbox a, Hitbox b, uint8_t side) {
  if (side == TOP) return a.y + a.h/2 - (b.y - b.y/2);
  if (side == RIGHT) return a.x + a.w/2 - (b.x - b.x/2);
  if (side == BOTTOM) return b.y + b.h/2 - (a.y - a.y/2);
  if (side == LEFT) return b.x + b.w/2 - (a.x - a.x/2);
}

