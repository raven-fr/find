#ifndef WORLD_H
#define WORLD_H

#include <SDL.h>

#define CHUNK_DIM 128

typedef enum tile {
	TILE_RED,
	TILE_GREEN,
	TILE_BLUE,
	TILE_LIGHT,
	MAX_COLLECTIBLE,

	TILE_EMPTY,
	TILE_WALL,
} tile;

extern SDL_Color tile_colors[];

#define PLAYER_STAMINA 1250
#define PLAYER_WALK_DELAY 100
#define PLAYER_PLACE_DELAY 175
#define PLAYER_GRAVITY 30
typedef struct player {
	SDL_Point pos;
	int scores[MAX_COLLECTIBLE];
	int stamina_time;
	int walk_wait;
	int place_wait;
	int gravity_time;
} player;

typedef struct chunk {
	SDL_Point pos;
	Uint8 tiles[CHUNK_DIM * CHUNK_DIM];
	SDL_Texture *tex;
	SDL_bool dirty;
	SDL_bool loaded;
	SDL_bool active;
} chunk;

#define MAX_CHUNKS 16
#define SAVE_INTERVAL 120000
typedef struct world {
	player player;
	SDL_Point view_pos;
	chunk chunks[MAX_CHUNKS];
	int save_time;
} world;

static inline SDL_Point chunk_pos_at(SDL_Point world_pos) {
	return (SDL_Point) {
		SDL_floor((double) world_pos.x / CHUNK_DIM),
		SDL_floor((double) world_pos.y / CHUNK_DIM),
	};
}

static inline SDL_Point pos_in_chunk(SDL_Point world_pos) {
	SDL_Point cpos = chunk_pos_at(world_pos);
	return (SDL_Point) {
		world_pos.x - cpos.x * CHUNK_DIM,
		world_pos.y - cpos.y * CHUNK_DIM,
	};
}

static inline int tile_index(SDL_Point pos) {
	SDL_assert(pos.x < CHUNK_DIM && pos.y < CHUNK_DIM);
	return pos.y * CHUNK_DIM + pos.x;
}

chunk *load_chunk(world *w, SDL_Point chunk_pos);
chunk *get_chunk(world *w, SDL_Point chunk_pos);
SDL_bool chunk_exists(world *w, SDL_Point chunk_pos);

tile get_tile(world *w, SDL_Point world_pos);
void set_tile(world *w, SDL_Point world_pos, tile t);

SDL_bool is_solid(tile t);

SDL_bool player_grounded(world *w);
void player_walk(world *w, int x, int y);
void player_place(world *w, int x, int y);
void player_destroy(world *w, int x, int y);
void player_collect(world *w, SDL_Point pos);

void tick_world(world *w);

#define VIEW_DIM 40
void draw_world(world *w, SDL_Window *win, SDL_Renderer *rend);

void init_world(world *w);

#endif
