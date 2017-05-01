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

#ifndef COMPONENTS_CGOL_H_
#define COMPONENTS_CGOL_H_

#define CGOL_MAX_GAMES 1

#include <stdint.h>

/* Opaque implementation pointer */
typedef struct cgol_s *cgol_t;

/* This will malloc 2*width*ceil(height/8) bytes for state buffers */
cgol_t cgol_init(int width, int height);

/* you must provide at least 2*width*ceil(height/8) bytes for storage */
cgol_t cgol_init_static(int width, int height, uint8_t *static_storage);

/* Get current state of the game. */
uint8_t* cgol_get_state(cgol_t ctx);

/* Perform a game turn */
void cgol_take_turn(cgol_t ctx);

/* Free any allocated memory and set ctx = NULL */
void cgol_free(cgol_t* ctx);

#endif /* COMPONENTS_CGOL_H_ */
