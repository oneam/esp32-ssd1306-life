/*
 * Conway's Game of Life implementation
 *
 * This implementation uses 2 bits of data for each cell of the game.
 * One is the current game state and one is used as temporary storage during turn-taking.
 * The state is stored in pages where each page is 8 rows of the board.
 *
 * The following code could be used to efficiently read the entire board:
 *
 * uint8_t* page = cgol_get_state(ctx);
 * uint8_t mask = 0x1;
 * for(int y=0; y < height; ++y) {
 *   for(int x=0; x < width; ++x) {
 *     bool is_live = (page[x] & mask) > 0;
 *   }
 *   mask <<= 1;
 *   if(mask == 0) {
 *     page += width;
 *     mask = 0x1;
 *   }
 * }
 *
 *  Copyright 2017 Sam Leitch
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "cgol.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct cgol_s {
  int width;
  int height;
  int num_pages;
  uint8_t* state;
  uint8_t* temp;
  uint8_t* internal_storage;
};

struct cgol_s games[CGOL_MAX_GAMES];
int games_alloced = 0;

static cgol_t init(int width, int height, uint8_t* static_storage) {
  if(width <= 0) return NULL;
  if(height <= 0) return NULL;
  if(games_alloced >= CGOL_MAX_GAMES) return NULL;

  cgol_t ctx = games + games_alloced;
  ++games_alloced;

  int num_pages = height >> 3;
  int page_partial = height & 0x7;
  if(page_partial) ++num_pages;

  uint8_t* internal_storage = NULL;
  if(static_storage == NULL) {
    internal_storage = (uint8_t*)malloc(2 * width * num_pages);
    if(!internal_storage) return NULL;
  }

  uint8_t* storage = internal_storage ? internal_storage : static_storage;

  ctx->width = width;
  ctx->height = height;
  ctx->num_pages = num_pages;
  ctx->internal_storage = internal_storage;
  ctx->state = storage;
  ctx->temp = storage + width * num_pages;

  return ctx;
}

cgol_t cgol_init(int width, int height) {
  return init(width, height, NULL);
}

cgol_t cgol_init_static(int width, int height, uint8_t *static_storage) {
  if(!static_storage) return NULL;
  return init(width, height, static_storage);
}

static bool apply_rules(uint8_t left_3_bits, uint8_t middle_3_bits, uint8_t right_3_bits) {
  bool is_living = (middle_3_bits & 0x2) > 0;

  int neighbors = 0;
  while(left_3_bits) {
    if(left_3_bits & 0x1) ++neighbors;
    left_3_bits >>= 1;
  }

  if(middle_3_bits & 0x1) ++neighbors;
  if(middle_3_bits & 0x4) ++neighbors;

  while(right_3_bits) {
    if(right_3_bits & 0x1) ++neighbors;
    right_3_bits >>= 1;
  }

  bool result = is_living;
  if(is_living && (neighbors < 2)) result = false; // under-population
  if(is_living && (neighbors > 3)) result = false; // over-population
  if(!is_living && (neighbors == 3)) result = true; // procreation
  return result;
}

static int get_3_bits(uint8_t* page, uint8_t* page_up, uint8_t* page_down, int x, int y) {
  int bit_offset = y & 0x7;
  uint8_t byte = page[x];
  if(bit_offset == 0) {
    uint8_t prev_byte = page_up ? page_up[x] : 0;
    return ((byte << 1) & 0x6) | ((prev_byte >> 7) & 0x1);
  } else if(bit_offset == 7) {
    uint8_t next_byte = page_down ? page_down[x] : 0;
    return ((byte >> 6) & 0x03) | ((next_byte << 2) & 0x4);
  } else {
    return (byte >> (bit_offset - 1)) & 0x7;
  }
}

void cgol_take_turn(cgol_t ctx) {
  memcpy(ctx->temp, ctx->state, ctx->num_pages * ctx->width);
  uint8_t* page = ctx->temp;
  uint8_t* page_up = NULL;
  uint8_t* page_down = page + ctx->width;
  uint8_t* new_page = ctx->state;
  uint8_t mask = 0x1;
  int page_index = 0;
  for(int y=0; y < ctx->height; ++y) {
    for(int x = 0; x < ctx->width; ++x) {
      uint8_t left_3_bits = x > 0 ? get_3_bits(page, page_up, page_down, x-1, y) : 0;
      uint8_t middle_3_bits = get_3_bits(page, page_up, page_down, x, y);
      uint8_t right_3_bits = x < (ctx->width - 1) ? get_3_bits(page, page_up, page_down, x+1, y) : 0;

      if(apply_rules(left_3_bits, middle_3_bits, right_3_bits)) {
        new_page[x] |= mask;
      } else {
        new_page[x] &= ~mask;
      }
    }

    mask <<= 1;
    if(mask == 0) {
      mask = 0x1;
      ++page_index;
      page += ctx->width;
      page_up = page - ctx->width;
      page_down = page_index < (ctx->num_pages - 1) ? page + ctx->width : NULL;
      new_page += ctx->width;
    }
  }
}

uint8_t* cgol_get_state(cgol_t ctx) {
  return ctx->state;
}

void cgol_free(cgol_t* ctx) {
  if(*ctx == NULL) return;
  free((*ctx)->internal_storage);
  *ctx = NULL;
}
