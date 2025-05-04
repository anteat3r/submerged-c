#include "globals.c"
#include <raylib.h>

typedef struct {
  BlockType *data;
  BlockPos pos;
} Chunk;

typedef struct {
  int num;
  Chunk *chunks;
} ChunkManager;

BlockType Chunk_GetBlockAtPos(Chunk chunk, BlockPos pos) {
  return chunk.data[CHUNK_WIDTH * pos.x + pos.y];
}

BlockType CM_GetBlockAtPos(ChunkManager *mng, BlockPos pos) {
  BlockPos chunk_pos = {
    pos.x / CHUNK_WIDTH,
    pos.y / CHUNK_HEIGHT,
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
