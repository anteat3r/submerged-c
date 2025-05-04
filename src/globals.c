#include "stdbool.h"
#include <stdint.h>
#include <raylib.h>
#include <stdint.h>

#define CHUNK_WIDTH 100
#define CHUNK_HEIGHT 100

typedef uint64_t BlockType;
#define BLOCK_EMPTY 0
#define BLOCK_SAND 1
#define BLOCK_GRAVEL 2

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
