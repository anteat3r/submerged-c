// INCLUDES

#include "raylib.h"
#include <cstdlib>
#include <emscripten/emscripten.h>
#include "stdbool.h"
#include <stdint.h>
#include <stdlib.h>

// DEFINES

#define FRAME_RATE 60
#define DELTA_TIME 1.0f / FRAME_RATE

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

#define PLAYER_WIDTH 2.f
#define PLAYER_HEIGHT 5.f

#define GRAVITY 0.1f

#define PLAYER_SPEED 0.05f
#define PLAYER_JUMP_POWER 10.f
#define PLAYER_DRAG 0.1f
// STRUCTS

typedef struct {
  VerletEntity Ventity;
  Hitbox hitbox;
  bool is_on_ground;
  bool can_swim;
  bool is_swimming;
} Player;

typedef struct {
  int x;
  int y;
} BlockPos;

typedef struct {
  int x;
  int y;
  int width;
  int height;
} BlockRect;

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
  int cap;
  Chunk *chunks;
} ChunkManager;

typedef struct {
  float x;
  float y;
  float lx;
  float ly;
} VerletEntity;

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

static ChunkManager manager = { 0, 0, NULL, };

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

bool IsXYSolid(float x, float y) {
  return IsPosSolid((Vector2){x, y});
}

// HITBOXES

bool IsHitboxColliding(Hitbox a, Hitbox b) {
  if (a.x <= b.x + b.w && a.x + a.w >= b.x && a.y <= b.y + b.h && a.y + a.h >= b.y) return true;
  return false;
}


/* gets the side(s) of hitbox A where it collides with hitbox B */
SideMask GetHitboxCollision(Hitbox a, Hitbox b) {
    SideMask res = 0;
    if (!IsHitboxColliding(a, b)) return res;

    // top edge of A lies within the vertical span of B
    if (a.y >= b.y && a.y <= b.y + b.h) res |= SIDE_TOP;

    // right edge of A lies within the horizontal span of B
    if (a.x + a.w >= b.x  && a.x + a.w <= b.x + b.w) res |= SIDE_RIGHT;

    // bottom edge of A lies within the vertical span of B
    if (a.y + a.h >= b.y && a.y + a.h <= b.y + b.h) res |= SIDE_BOTTOM;

    // left edge of A lies within the horizontal span of B
    if (a.x >= b.x && a.x <= b.x + b.w) res |= SIDE_LEFT;

    return res;
}

float GetHitboxOverlap(Hitbox a, Hitbox b, uint8_t side) {
  if (side == SIDE_TOP)    return (a.y + a.h) - b.y;
  if (side == SIDE_RIGHT)  return (a.x + a.w) - b.x;
  if (side == SIDE_BOTTOM) return (b.y + b.h) - a.y;
  if (side == SIDE_LEFT)   return (b.x + b.w) - a.x;
}

Hitbox BP_GetHitbox(BlockPos *pos) {
  return (Hitbox){
    .x = pos->x,
    .y = pos->y,
    .w = 1.f,
    .h = 1.f
  };
}


// VERLET ENTITY

void VE_Tick (VerletEntity *entity) {
  float dx = entity->x - entity->lx;
  float dy = entity->y - entity->ly;
  entity->lx = entity->x;
  entity->ly = entity->y;
  entity->x += dx;
  entity->y += dy;
}

void VE_MoveBy (VerletEntity *entity, float x, float y) {
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

void VE_GetSpeed(VerletEntity *entity, float *x, float *y) {
  *x = (entity->x - entity->lx) * DELTA_TIME;
  *y = (entity->y - entity->ly) * DELTA_TIME;
}

void VE_SetSpeed(VerletEntity *entity, float x, float y) {
  entity->lx = entity->x - x * DELTA_TIME;
  entity->ly = entity->y - y * DELTA_TIME;
}

// CHUNKS

BlockType Chunk_GetBlockAtPos(Chunk chunk, BlockPos pos) {
  return chunk.data[CHUNK_WIDTH * pos.x + pos.y];
}

BlockPos GetChunkPos(BlockPos pos) {
  return (BlockPos){
    pos.x / CHUNK_WIDTH - (pos.x < 0 ? 1 : 0),
    pos.y / CHUNK_HEIGHT - (pos.y < 0 ? 1 : 0),
  };
}

BlockPos GetPosInChunk(BlockPos pos) {
  return (BlockPos){
    pos.x % CHUNK_WIDTH + (pos.x < 0 ? CHUNK_WIDTH : 0),
    pos.y % CHUNK_HEIGHT + (pos.y < 0 ? CHUNK_HEIGHT : 0),
  };
}

Chunk GetChunkAtPos(BlockPos pos) {
  for (int i = 0; i < manager.num; i++) {
    Chunk chunk = manager.chunks[i];
    if (chunk.pos.x == pos.x && chunk.pos.y == pos.y) {
      return chunk;
    }
  }
  return (Chunk){ .data = NULL };
}

int GetChunkIdxAtPos(BlockPos pos) {
  for (int i = 0; i < manager.num; i++) {
    Chunk chunk = manager.chunks[i];
    if (chunk.pos.x == pos.x && chunk.pos.y == pos.y) {
      return i;
    }
  }
  return -1;
}

BlockType GetBlockAtPos(BlockPos pos) {
  BlockPos chunk_pos = GetChunkPos(pos);
  Chunk chunk = GetChunkAtPos(chunk_pos);
  if (chunk.data == NULL) return -1;
  return Chunk_GetBlockAtPos(chunk, GetPosInChunk(pos));
}

void FillBlockArray(BlockType *arr, BlockRect rect) {
  BlockPos topleft_chunk = GetChunkPos((BlockPos){
    rect.x, rect.y,
  });
  BlockPos botright_chunk = GetChunkPos((BlockPos){
    rect.x + rect.width, rect.y + rect.height,
  });
  BlockPos rect_size = {
    botright_chunk.x - topleft_chunk.x + 1,
    botright_chunk.y - topleft_chunk.y + 1,
  };
  Chunk *chunk_arr = malloc(rect_size.x * rect_size.y * sizeof(Chunk*));
  if (chunk_arr == NULL) return;
  for (int i = 0; i < rect_size.x; i++) {
    for (int j = 0; j < rect_size.y; j++) {
      chunk_arr[i * rect_size.x + j] = GetChunkAtPos((BlockPos){i, j});
    }
  }
  for (int i = 0; i < rect.width; i++) {
    for (int j = 0; j < rect.height; j++) {
      BlockPos pos = {rect.x + i, rect.y + j};
      BlockPos ch_pos = GetChunkPos(pos);
      Chunk chunk = chunk_arr[ch_pos.x * rect_size.x + ch_pos.y];
      if (chunk.data == NULL) {
        arr[i * rect.width + j] = -1;
        continue;
      }
      arr[i * rect.width + j] = Chunk_GetBlockAtPos(chunk, pos);
    }
  }
  free(chunk_arr);
}

void AddChunk(Chunk chunk) {
  manager.num++;
  if (manager.chunks == NULL) {
    manager.chunks = malloc(sizeof(Chunk));
    manager.cap = 1;
  } else if (manager.num < manager.cap) {
    manager.chunks = realloc(manager.chunks, manager.num);
    manager.cap = manager.num;
  }
  manager.chunks[manager.num - 1] = chunk;
}

void RemoveChunk(BlockPos pos) {
  int idx = GetChunkIdxAtPos(pos);
  if (idx == -1) return;
  Chunk chunk = manager.chunks[idx];
  free(chunk.data);
  manager.num--;
  for (int i = idx; i < manager.num; i++) {
    manager.chunks[i] = manager.chunks[i + 1];
  }
}

Chunk CreateEmptyChunk(BlockPos pos) {
  return (Chunk){
    .data = malloc(CHUNK_WIDTH * CHUNK_HEIGHT * sizeof(BlockType)),
    .pos = pos,
  };
}


// PLAYER
void P_Tick(Player *player) {
  // update state variables

  // get the surrounding blocks
  float p_top = player->Ventity.y;
  float p_bottom = player->Ventity.y + PLAYER_HEIGHT;
  float p_left = player->Ventity.x;
  float p_right = player->Ventity.x + PLAYER_WIDTH;

  BlockRect rect = {
    .x = (int)p_left - 1,
    .y = (int)p_top - 1,
    .width = (int)p_right - (int)p_left + 3,
    .height = (int)p_bottom - (int)p_top + 3,
  };

  blocks = (BlockType*)malloc(rect.width * rect.height * sizeof(BlockType));
  if (blocks == NULL) {
    printf("Failed to allocate memory for surround blocks in P_Tick()\n");
    return;
  }

  FillBlockArray(blocks, rect);

  player->is_on_ground = false;


  // resolve collisions
  for (int i = 0; i < rect.width; i++) {
    for (int j = 0; j < rect.height; j++) {
      int x = rect.x + i;
      int y = rect.y + j;
      BlockPos pos = {x, y};
      Hitbox hitbox = BP_GetHitbox(&pos);

      if (blocks[i * rect.width + j] != BLOCK_EMPTY) {
        if (IsHitboxColliding(player->hitbox, hitbox)) {
          // fix the smallest overlap
          float overlaps[4];
          uint smallest_overlap_idx = 0;
          uint biggest_overlap_idx = 0;
          overlaps[0] = GetHitboxOverlap(player->hitbox, hitbox, SIDE_TOP);
          overlaps[1] = GetHitboxOverlap(player->hitbox, hitbox, SIDE_RIGHT);
          overlaps[2] = GetHitboxOverlap(player->hitbox, hitbox, SIDE_BOTTOM);
          overlaps[3] = GetHitboxOverlap(player->hitbox, hitbox, SIDE_LEFT);
          float min_overlap = overlaps[0];
          float max_overlap = overlaps[2]; // side bottom has priority in a tie
          for (int k = 0; k < 4; k++) {
            if (overlaps[k] < min_overlap) {
              min_overlap = overlaps[k];
              smallest_overlap_idx = k;
            }
            if (overlaps[k] > max_overlap) {
              max_overlap = overlaps[k];
              biggest_overlap_idx = k;
            }
          }
          
          if (smallest_overlap_idx == 0) {
            VE_MoveBy(&player->Ventity, 0, min_overlap);
          }
          else if (smallest_overlap_idx == 1) {
            VE_MoveBy(&player->Ventity, min_overlap, 0);
          }
          else if (smallest_overlap_idx == 2) {
            VE_MoveBy(&player->Ventity, 0, -min_overlap);
          }
          else {
            VE_MoveBy(&player->Ventity, -min_overlap, 0);
          }
          
          VE_SetSpeed(&player->Ventity, 0, 0);
          
          if (biggest_overlap_idx == 2) {  // if the player's bottom of its hitbox is touching some block
            player->is_on_ground = true;
          }
        }
      }




      blocks[i * rect.width + j]
    }
  }


  VE_ApplyForce(&player->Ventity, 0, GRAVITY);  // apply gravity

  if (IsKeyDown(KEY_W) && player->is_on_ground) {
    VE_ApplyForce(&player->Ventity, 0, -PLAYER_JUMP_POWER);
  }
  if (IsKeyDown(KEY_A)) {
    VE_ApplyForce(&player->Ventity, -PLAYER_SPEED, 0);
  }
  if (IsKeyDown(KEY_D)) {
    VE_ApplyForce(&player->Ventity, PLAYER_SPEED, 0);
  }

  // apply drag
  float speed_x;
  float speed_y;
  VE_GetSpeed(&player->Ventity, &speed_x, &speed_y);

  VE_ApplyForce(&player->Ventity, -speed_x * PLAYER_DRAG, -speed_y * PLAYER_DRAG);
  VE_Tick(&player->Ventity);

  free(blocks);
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

  Vector2 topleft = GetScreenToWorld2D((Vector2){0, 0}, camera);
  Vector2 bottomright = GetScreenToWorld2D(
    (Vector2){GetScreenWidth(), GetScreenHeight()},
    camera
  );

  BlockRect rect = {
    topleft.x - 1,
    topleft.y - 1,
    bottomright.x - topleft.x + 1,
    bottomright.y - topleft.y + 1,
  };

  BlockType *arr = malloc(rect.width * rect.height * sizeof(BlockType));
  FillBlockArray(arr, rect);
  for (int i = 0; i < rect.width; i++) {
    for (int j = 0; j < rect.height; j++) {
      BlockType btype = arr[i * rect.width + j];
      DrawRectangleV(
        (Vector2){rect.x + i, rect.y + j},
        (Vector2){1, 1},
        btype == BLOCK_EMPTY ? DARKBLUE : RED
      );
    }
  }
  free(arr);

  DrawRectangleV((Vector2){
    player.Ventity.x,
    player.Ventity.y,
  }, (Vector2){
    PLAYER_WIDTH,
    PLAYER_HEIGHT,
  }, GREEN);

  EndMode2D();
  EndDrawing();
}

int main() {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(300, 300, "Submerged");

  Chunk chunk = CreateEmptyChunk((BlockPos){0, 0});
  chunk.data[33] = BLOCK_SAND;
  AddChunk(chunk);

  emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
  CloseWindow();
  free(manager.chunks);
}
