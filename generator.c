#include <SDL.h>

#include "world.h"
#include "random.h"

#define MAX_INFLUENCE 10
#define SPARSITY 40.0
#define CAVERNITY 3

#define SURFACE_COLLECTIBLE_RARITY 70

void generate_chunk(world *w, chunk *c) {
	SDL_memset(c->tiles, TILE_EMPTY, sizeof(c->tiles));
	if (c->pos.y < 0) return;

	static float noise_params[CHUNK_DIM * CHUNK_DIM];
	SDL_memset(noise_params, 0, sizeof(noise_params));

	for (int i = 0; i < CHUNK_DIM * CHUNK_DIM; i++) {
		if (rand_int() % CAVERNITY == 0) {
			noise_params[i] =
				SDL_pow(rand_float(), SPARSITY) * MAX_INFLUENCE;
		}
	}

	for (int y = 0; y < CHUNK_DIM; y++) {
		for (int x = 0; x < CHUNK_DIM; x++) {
			float influence = noise_params[tile_index((SDL_Point) {x, y})];
			if (c->pos.y == 0) {
				if (y < 96) influence = 0;
				else influence *= 1.8;
			}

			int boundary = influence + 0.5;
			int min_x = SDL_max(x - boundary, 0);
			int max_x = SDL_min(x + boundary, CHUNK_DIM - 1);
			int min_y = SDL_max(y - boundary, 0);
			int max_y = SDL_min(y + boundary, CHUNK_DIM - 1);

			for (int iy = min_y; iy <= max_y; iy++) {
				for (int ix = min_x; ix <= max_x; ix++) {
					int dx = ix - x;
					int dy = iy - y;
					if (dx * dx + dy * dy <= influence * influence) {
						if (dx != 0 || dy != 0 || influence > 0.5) {
							c->tiles[
								tile_index((SDL_Point) {ix, iy})] = TILE_WALL;
						}
					}
				}
			}
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
