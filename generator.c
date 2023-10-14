#include <SDL.h>

#include "world.h"
#include "random.h"

#define FULLNESS 15.0
#define SPARSITY 40.0
#define CAVERNITY 20.0

#define SURFACE_COLLECTIBLE_RARITY 50

static void fill_circle(Uint8 tiles[], int x, int y, float radius, tile t) {
	int boundary = SDL_fabs(radius) + 0.5;
	int min_x = SDL_max(x - boundary, 0);
	int max_x = SDL_min(x + boundary, CHUNK_DIM - 1);
	int min_y = SDL_max(y - boundary, 0);
	int max_y = SDL_min(y + boundary, CHUNK_DIM - 1);
	for (int iy = min_y; iy <= max_y; iy++) {
		for (int ix = min_x; ix <= max_x; ix++) {
			int dx = ix - x;
			int dy = iy - y;
			if (dx * dx + dy * dy <= radius * radius) {
				if (dx != 0 || dy != 0 || radius > 0.5) {
					tiles[tile_index((SDL_Point) {ix, iy})] = t;
				}
			}
		}
	}
}

void generate_chunk(world *w, chunk *c) {
	SDL_memset(c->tiles, TILE_EMPTY, sizeof(c->tiles));
	if (c->pos.y < 0) return;

	for (int y = 0; y < CHUNK_DIM; y++) {
		for (int x = 0; x < CHUNK_DIM; x++) {
			float radius = SDL_pow(rand_float(), SPARSITY);
			if (rand_int() % 4 == 0) radius = -radius;
			radius *= (radius > 0) ? FULLNESS : CAVERNITY;
			if (c->pos.y == 0) {
				if (y < 96) radius = 0;
				else if (radius > 0) radius *= 1.8;
			}

			tile t = radius > 0 ? TILE_WALL : TILE_EMPTY;
			fill_circle(c->tiles, x, y, SDL_abs(radius), t);
		}
	}

	int rarity = c->pos.x != 0 && c->pos.x != -1 ?
		SURFACE_COLLECTIBLE_RARITY : 8;
	if (c->pos.y == 0) {
		for (int x = 0; x < CHUNK_DIM; x++) {
			if (rand_int() % rarity == 0) {
				int y = 0;
				while (!is_solid(c->tiles[tile_index((SDL_Point) {x, y + 1})]))
					y++;
				tile t = rand_int() % TILE_LIGHT;
				c->tiles[tile_index((SDL_Point) {x, y})] = t;
			}
		}
	}
}
