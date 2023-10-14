#include <SDL.h>

#include "world.h"
#include "random.h"
#include "generator.h"
#include "save.h"
#include "counter.h"

SDL_Color tile_colors[] = {
	[TILE_RED] = {0xFF, 0x00, 0x00, 0xFF},
	[TILE_GREEN] = {0x00, 0xFF, 0x00, 0xFF},
	[TILE_BLUE] = {0x00, 0x00, 0xFF, 0xFF},
	[TILE_LIGHT] = {0xFF, 0xFF, 0xFF, 0xFF},
	[TILE_EMPTY] = {0x00, 0x00, 0x00, 0xFF},
	[TILE_WALL] = {0x40, 0x40, 0x40, 0xFF},
	[TILE_BLOCK_WHITE] = {0x80, 0x80, 0x80, 0xFF},
};

chunk *get_chunk(world *w, SDL_Point chunk_pos) {
	for (int i = 0; i < MAX_CHUNKS; i++) {
		chunk *c = &w->chunks[i];
		if (!c->loaded) continue;
		if (c->pos.x == chunk_pos.x && c->pos.y == chunk_pos.y) {
			c->active = SDL_TRUE;
			return c;
		}
	}
	return NULL;
}

SDL_bool chunk_exists(world *w, SDL_Point chunk_pos) {
	if (get_chunk(w, chunk_pos)) return SDL_TRUE;
	if (is_chunk_saved(w, chunk_pos)) return SDL_TRUE;
	return SDL_FALSE;
}

static void unload_chunk(world *w, chunk *c) {
	save_chunk(w, c);
	c->active = SDL_FALSE;
	c->loaded = SDL_FALSE;
}

static void deactivate_chunk(chunk *c) {
	c->active = SDL_FALSE;
}

chunk *load_chunk(world *w, SDL_Point chunk_pos) {
	chunk *c = get_chunk(w, chunk_pos);
	if (c) return c;

	static int last_loaded = 0;
	for (int i = 0; i < MAX_CHUNKS; i++) {
		chunk *wc = &w->chunks[(last_loaded + i) % MAX_CHUNKS];
		if (!wc->active) {
			last_loaded = i;
			c = wc;
			break;
		}
	}
	if (!c) return NULL;

	if (c->loaded) unload_chunk(w, c);

	c->pos = chunk_pos;
	c->active = SDL_TRUE;
	c->dirty = SDL_TRUE;
	c->loaded = SDL_TRUE;
	if (!load_saved_chunk(w, c))
		generate_chunk(w, c);
	
	return c;
}

SDL_Texture *render_chunk(SDL_Renderer *rend, chunk *c) {
	if (!c->tex) {
		c->tex = SDL_CreateTexture(rend,
			SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
			CHUNK_DIM, CHUNK_DIM);
		SDL_assert(c->tex);
		c->dirty = SDL_TRUE;
	}

	if (c->dirty) {
		int pitch;
		void *out;
		SDL_assert(SDL_LockTexture(c->tex, NULL, &out, &pitch) >= 0);
		SDL_Color *pixels = out;

		for (int i = 0; i < CHUNK_DIM * CHUNK_DIM; i++) {
			tile t = c->tiles[i];
			SDL_Color color = tile_colors[t];
			if (t == TILE_WALL) {
				int variance = hash(i) % 5;
				color.r += variance;
				color.g += variance;
				color.b += variance;
			}
			pixels[i] = color;
		}

		SDL_UnlockTexture(c->tex);
		c->dirty = SDL_FALSE;
	}
	return c->tex;
}

tile get_tile(world *w, SDL_Point world_pos) {
	chunk *c = load_chunk(w, chunk_pos_at(world_pos));
	SDL_assert(c);
	return c->tiles[tile_index(pos_in_chunk(world_pos))];
}

void set_tile(world *w, SDL_Point world_pos, tile t) {
	chunk *c = load_chunk(w, chunk_pos_at(world_pos));
	SDL_assert(c);
	c->tiles[tile_index(pos_in_chunk(world_pos))] = t;
	c->dirty = SDL_TRUE;
}

SDL_bool is_solid(tile t) {
	switch (t) {
	case TILE_WALL:
	case TILE_BLOCK_WHITE:
		return SDL_TRUE;
	default:
		return SDL_FALSE;
	}
}

SDL_bool player_grounded(world *w) {
	if (SDL_TICKS_PASSED(SDL_GetTicks(), w->player.stamina_time))
		return SDL_FALSE;
	SDL_bool grounded = SDL_FALSE;
	for (int y = w->player.pos.y; y < w->player.pos.y + 2; y++) {
		for (int x = w->player.pos.x - 1; x < w->player.pos.x + 2; x++)
			grounded = grounded || is_solid(get_tile(w, (SDL_Point) {x, y}));
	}
	return grounded;
}

void player_walk(world *w, int x, int y) {
	if (!SDL_TICKS_PASSED(SDL_GetTicks(), w->player.walk_wait))
		return;

	if (x != 0 || y != 0) {
		SDL_Point new_pos = w->player.pos;
		new_pos.x += x;
		if (player_grounded(w))
			new_pos.y += y;
		if (is_solid(get_tile(w, new_pos))) {
			if (y == 0 && player_grounded(w)) {
				new_pos = w->player.pos;
				new_pos.y -= 1;
				if (!is_solid(get_tile(w, new_pos)))
					w->player.pos = new_pos;
			}
		} else w->player.pos = new_pos;
		w->player.walk_wait = SDL_GetTicks() + PLAYER_WALK_DELAY;
	}
}

void player_collect(world *w, SDL_Point pos) {
	tile t = get_tile(w, pos);
	if (t < MAX_COLLECTIBLE) {
		w->player.scores[t] += 1;
		set_tile(w, pos, TILE_EMPTY);
	}
}

void player_place(world *w, int x, int y) {
	if (!player_grounded(w)) return;
	if (!SDL_TICKS_PASSED(SDL_GetTicks(), w->player.place_wait)) return;
	w->player.place_wait = SDL_GetTicks() + PLAYER_PLACE_DELAY;
	if (w->player.scores[TILE_LIGHT] < 1) return;

	SDL_Point place = {w->player.pos.x + x, w->player.pos.y + y};
	if (!is_solid(get_tile(w, place))) {
		player_collect(w, place);
		set_tile(w, place, TILE_BLOCK_WHITE);
		w->player.scores[TILE_LIGHT]--;
	} else {
		SDL_Point push = {w->player.pos.x - x, w->player.pos.y - y};
		if (!is_solid(get_tile(w, push))) {
			set_tile(w, w->player.pos, TILE_BLOCK_WHITE);
			w->player.pos = push;
			w->player.scores[TILE_LIGHT]--;
		}
	}
}


void player_destroy(world *w, int x, int y) {
	if (!SDL_TICKS_PASSED(SDL_GetTicks(), w->player.place_wait)) return;

	SDL_Point destroy = {w->player.pos.x + x, w->player.pos.y + y};
	tile t = get_tile(w, destroy);

	if (t != TILE_EMPTY) {
		if (t < MAX_COLLECTIBLE) {
			player_collect(w, destroy);
		} else {
			if (w->player.scores[TILE_LIGHT] < 1) return;
			w->player.scores[TILE_LIGHT]--;

			set_tile(w, destroy, TILE_EMPTY);
			w->player.place_wait = SDL_GetTicks() + PLAYER_PLACE_DELAY;
		}
	}
}

static int get_view_scale(SDL_Window *win) {
	int sw, sh;
	SDL_GetWindowSize(win, &sw, &sh);

	int view_scale;
	if (sh < sw)
		view_scale = sh / VIEW_DIM;
	else
		view_scale = sw / VIEW_DIM;

	return view_scale;
}

static void draw_ui(world *w, SDL_Window *win, SDL_Renderer *rend) {
	int sw, sh;
	SDL_GetWindowSize(win, &sw, &sh);
	int view_scale = get_view_scale(win);

	int bar_size = 1.5 * view_scale;
	int padding = 0.24 * view_scale;

	SDL_Rect bar = {
		0, sh - (bar_size + padding), sw, bar_size + padding,
	};
	SDL_SetRenderDrawColor(rend, 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderFillRect(rend, &bar);

	int x = padding;
	for (int i = 0; i < MAX_COLLECTIBLE; i++) {
		int score = w->player.scores[i];
		SDL_Color grey = {0x20, 0x20, 0x10, 0xFF};
		SDL_Color collectible_color = tile_colors[i];
		SDL_Color color = score > 0 ? collectible_color : grey;
		SDL_Point pos = {x, bar.y + padding};
		x += draw_counter(rend, pos, bar_size, color, score, 3);
		x += view_scale * 1;
	}
}

void draw_world(world *w, SDL_Window *win, SDL_Renderer *rend) {
	int sw, sh;
	SDL_GetWindowSize(win, &sw, &sh);
	int view_scale = get_view_scale(win);

	int view_width = sw / view_scale;
	int view_height = sh / view_scale;
	int view_x = w->view_pos.x - view_width / 2;
	int view_y = w->view_pos.y - view_height / 2;

	SDL_Rect view_rect = {view_x, view_y, view_width + 1, view_height + 1};
	for (int i = 0; i < MAX_CHUNKS; i++) {
		chunk *c = &w->chunks[i];
		if (c->active) {
			SDL_Rect chunk_rect = {
				c->pos.x * CHUNK_DIM, c->pos.y * CHUNK_DIM,
				CHUNK_DIM, CHUNK_DIM,
			};
			if (!SDL_HasIntersection(&view_rect, &chunk_rect))
				deactivate_chunk(c);
		}
	}
	
	SDL_Point min = chunk_pos_at((SDL_Point) {view_x, view_y});
	SDL_Point max = chunk_pos_at(
		(SDL_Point) {view_x + view_width, view_y + view_height});
	for (int y = min.y - 1; y <= max.y; y++) {
		for (int x = min.x - 1; x <= max.x; x++) {
			chunk *c = load_chunk(w, (SDL_Point) {x, y});
			if (!c) continue;
			SDL_Texture *tex = render_chunk(rend, c);

			int world_x = x * CHUNK_DIM;
			int world_y = y * CHUNK_DIM;
			int draw_x = (world_x - view_x) * view_scale;
			int draw_y = (world_y - view_y) * view_scale;
			SDL_Rect draw_rect = {
				draw_x, draw_y, CHUNK_DIM * view_scale, CHUNK_DIM * view_scale,
			};
			SDL_RenderCopy(rend, tex, NULL, &draw_rect);
		}
	}

	int player_x = (w->player.pos.x - view_x) * view_scale;
	int player_y = (w->player.pos.y - view_y) * view_scale;
	SDL_Rect player_rect = {
		player_x, player_y, view_scale, view_scale,
	};
	SDL_SetRenderDrawColor(rend, 0xFF, 0x00, 0xFF, 0xFF);
	SDL_RenderFillRect(rend, &player_rect);

	draw_ui(w, win, rend);
}

void tick_world(world *w) {
	get_rand(); // shuffle RNG
	
	player_collect(w, w->player.pos);
	int to_combine = SDL_min(w->player.scores[TILE_RED],
		SDL_min(w->player.scores[TILE_GREEN], w->player.scores[TILE_BLUE]));
	w->player.scores[TILE_LIGHT] += to_combine;
	w->player.scores[TILE_RED] -= to_combine;
	w->player.scores[TILE_GREEN] -= to_combine;
	w->player.scores[TILE_BLUE] -= to_combine;

	tile below = get_tile(w,
		(SDL_Point) {w->player.pos.x, w->player.pos.y + 1});
	if (is_solid(below)) 
		w->player.stamina_time = SDL_GetTicks() + PLAYER_STAMINA;

	if (!player_grounded(w)) {
		if (SDL_TICKS_PASSED(SDL_GetTicks(), w->player.gravity_time)) {
			SDL_Point new_pos = {w->player.pos.x, w->player.pos.y + 1};
			if (!is_solid(get_tile(w, new_pos)))
				w->player.pos = new_pos;
			w->player.gravity_time = SDL_GetTicks() + PLAYER_GRAVITY;
		}
	}

	if (player_grounded(w)) {
		if (w->view_pos.x < w->player.pos.x)
			w->view_pos.x += 1;
		else if (w->view_pos.x > w->player.pos.x)
			w->view_pos.x -= 1;
		if (w->view_pos.y < w->player.pos.y)
			w->view_pos.y += 1;
		else if (w->view_pos.y > w->player.pos.y)
			w->view_pos.y -= 1;
	} else {
		if (w->view_pos.x < w->player.pos.x - VIEW_DIM / 3)
			w->view_pos.x += 1;
		else if (w->view_pos.x > w->player.pos.x + VIEW_DIM / 3)
			w->view_pos.x -= 1;
		if (w->view_pos.y < w->player.pos.y - VIEW_DIM / 3)
			w->view_pos.y += 1;
		else if (w->view_pos.y > w->player.pos.y + VIEW_DIM / 3)
			w->view_pos.y -= 1;
	}

	if (SDL_TICKS_PASSED(SDL_GetTicks(), w->save_time)) {
		save_world(w);
		w->save_time = SDL_GetTicks() + SAVE_INTERVAL;
	}
}

void init_world(world *w) {
	if (!load_world(w)) {
		w->player.pos.x += rand_int() % 50 - 25;
		w->player.scores[TILE_LIGHT] = 5;
	}
	w->view_pos = w->player.pos;
}
