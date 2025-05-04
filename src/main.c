// INCLUDES

#include "raylib.h"
#include <emscripten/emscripten.h>
#include "stdbool.h"
#include <stdint.h>

// DEFINES

#define CHUNK_WIDTH 100
#define CHUNK_HEIGHT 100

typedef uint64_t BlockType;
#define BLOCK_EMPTY 0
#define BLOCK_WATER 1
#define BLOCK_SAND 2
#define BLOCK_GRAVEL 3

typedef uint8_t SideMask;
#define SIDE_TOP 1
#define SIDE_RIGHT 2
#define SIDE_BOTTOM 4
#define SIDE_LEFT 8

// STRUCTS

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

typedef struct {
  BlockType *data;
  BlockPos pos;
} Chunk;

typedef struct {
  int num;
  Chunk *chunks;
} ChunkManager;

// STATICS

static Player player = {
  .pos = (Vector2){0, 0},
  .last_pos = (Vector2){0, 0},
};

static Camera2D camera = {
  (Vector2){0, 0},
  (Vector2){0, 0},
  0, 1,
};

static uint64_t frame_count = 0;


// FUNCTIONS


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

// HITBOXES

bool IsHitboxColliding(Hitbox a, Hitbox b) {
  if (a.x <= b.x + b.w && a.x + a.w >= b.x && a.y <= b.y + b.h && a.y + a.h >= b.y) return true;
  return false;
}

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

// CHUNKS

BlockType Chunk_GetBlockAtPos(Chunk chunk, BlockPos pos) {
  return chunk.data[CHUNK_WIDTH * pos.x + pos.y];
}

BlockPos GetChunkPos(BlockPos pos, BlockPos chunk) {
  // return 
}

BlockType CM_GetBlockAtPos(ChunkManager *mng, BlockPos pos) {
  BlockPos chunk_pos = {
    pos.x / CHUNK_WIDTH - (pos.x < 0 ? 1 : 0),
    pos.y / CHUNK_HEIGHT - (pos.y < 0 ? 1 : 0),
  };
  for (int i = 0; i < mng->num; i++) {
    Chunk chunk = mng->chunks[i];
    if (chunk.pos.x == chunk_pos.x && chunk.pos.y == chunk_pos.y) {
      return Chunk_GetBlockAtPos(chunk, (BlockPos){
        pos.x % CHUNK_WIDTH,
        pos.y % CHUNK_HEIGHT,
      });
    }
  }
  return -1;
}

int CM_FillBlockArray(ChunkManager *mng, BlockType *arr, Rectangle rect) {

}

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

int main() {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(300, 300, "Submerged");
  emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
  CloseWindow();
}
