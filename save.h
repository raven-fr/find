#ifndef SAVE_H
#define SAVE_H

#include <SDL.h>

#include "world.h"

void save_chunk(world *w, chunk *c);
SDL_bool is_chunk_saved(world *w, SDL_Point chunk_pos);
SDL_bool load_saved_chunk(world *w, chunk *c);
SDL_bool load_world(world *w);
void save_world(world *w);

#endif
