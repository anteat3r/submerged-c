#include "stdbool.h"
#include <raylib.h>
#include <stdint.h>

#define CHUNK_WIDTH 100
#define CHUNK_HEIGHT 100
#define FRAME_RATE 60
#define DELTA_TIME 1.f/FRAME_RATE

typedef uint8_t SideMask;

#define TOP    1
#define RIGHT  2
#define BOTTOM 4
#define LEFT   8


typedef struct {
  Vector2 pos;
  Vector2 last_pos;
} Player;

typedef struct {
  int x;
  int y;
} BlockPos;

typedef struct {
  float x;
  float y;
  float w;
  float h;
} Hitbox;

Hitbox BP_GetHitbox(BlockPos *pos) {
  return (Hitbox){
    .x = pos->x,
    .y = pos->y,
    .w = 1.f,
    .h = 1.f
  }
}

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


bool IsHitboxColliding(Hitbox a, Hitbox b) {
  if (a.x <= b.x + b.w && a.x + a.w >= b.x && a.y <= b.y + b.h && a.y + a.h >= b.y) return true;
  return false;
}


/* gets the side(s) of hitbox a where it collides with hitbox b */
SideMask GetHitboxCollision(Hitbox a, Hitbox b) {
    SideMask res = 0;
    if (!IsHitboxColliding(a, b)) return res;

    // top edge of A lies within the vertical span of B
    if (a.y >= b.y && a.y <= b.y + b.h) res |= TOP;

    // right edge of A lies within the horizontal span of B
    if (a.x + a.w >= b.x  && a.x + a.w <= b.x + b.w) res |= RIGHT;

    // bottom edge of A lies within the vertical span of B
    if (a.y + a.h >= b.y && a.y + a.h <= b.y + b.h) res |= BOTTOM;

    // left edge of A lies within the horizontal span of B
    if (a.x >= b.x && a.x <= b.x + b.w) res |= LEFT;

    return res;
}

float GetHitboxOverlap(Hitbox a, Hitbox b, uint8_t side) {
  if (side == TOP)    return (a.y + a.h) - b.y;
  if (side == RIGHT)  return (a.x + a.w) - b.x;
  if (side == BOTTOM) return (b.y + b.h) - a.y;
  if (side == LEFT)   return (b.x + b.w) - a.x;
}

typedef struct {
  float x;
  float y;
  float lx;
  float ly;
} VerletEntity;

void VE_Tick (VerletEntity *entity) {
  float dx = entity->x - entity->lx;
  float dy = entity->y - entity->ly;
  entity->lx = entity->x;
  entity->ly = entity->y;
  entity->x += dx;
  entity->y += dy;
}

void VE_Move (VerletEntity *entity, float x, float y) {
  entity->x += x;
  entity->y += y;
}

void VE_SetPos (VerletEntity *entity, float x, float y) {
  entity->x = x;
  entity->y = y;
}

void VE_ApplyForce (VerletEntity *entity, float x, float y) {
  entity->lx -= x;
  entity->ly -= y;
}