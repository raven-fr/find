#include <SDL.h>

#include "world.h"
#include "die.h"

typedef char filename[4096];

#define MAGIC 0xBEE10D5
#define SCHEMA 0

static const char *pref_path() {
	static char *path = NULL;
	if (!path) path = SDL_GetPrefPath("citrons", "findgame");
	sdl_error_assert(path);
	return path;
}

static void get_chunk_filename(SDL_Point chunk_pos, filename name) {
	int written = SDL_snprintf(name, sizeof(filename), "%schunk.%d.%d.dat",
		pref_path(), chunk_pos.x, chunk_pos.y);
	if (written >= sizeof(filename)) die("filename too large\n");
}

static void get_world_filename(filename name) {
	int written = SDL_snprintf(name, sizeof(filename), "%sworld.dat", pref_path());
	if (written >= sizeof(filename)) die("filename too large\n");
}

static void write_header(SDL_RWops *file) {
	Uint32 magic = MAGIC;
	Uint32 schema = 0;
	sdl_error_assert(SDL_RWwrite(file, &magic, sizeof(magic), 1));
	sdl_error_assert(SDL_RWwrite(file, &schema, sizeof(schema), 1));
}

static void read_header(SDL_RWops *file) {
	Uint32 magic;
	Uint32 schema;
	sdl_error_assert(SDL_RWread(file, &magic, sizeof(magic), 1));
	if (magic != MAGIC) die("invalid save file");
	sdl_error_assert(SDL_RWread(file, &schema, sizeof(schema), 1));
	if (schema != SCHEMA) die("invalid save file");
}

void save_chunk(world *w, chunk *c) {
	SDL_assert(c->loaded);

	filename name;
	get_chunk_filename(c->pos, name);

	SDL_RWops *file = SDL_RWFromFile(name, "wb");
	sdl_error_assert(file);
	write_header(file);
	
	sdl_error_assert(
		SDL_RWwrite(file, c->tiles, sizeof(c->tiles), 1));

	sdl_error_assert(SDL_RWclose(file) >= 0);
}

void save_world(world *w) {
	filename name;
	get_world_filename(name);

	SDL_RWops *file = SDL_RWFromFile(name, "wb");
	sdl_error_assert(file);
	write_header(file);

	sdl_error_assert(
		SDL_RWwrite(file, &w->player.pos, sizeof(w->player.pos), 1));
	sdl_error_assert(
		SDL_RWwrite(file, &w->player.scores, sizeof(w->player.scores), 1));
	
	sdl_error_assert(SDL_RWclose(file) >= 0);

	for (int i = 0; i < MAX_CHUNKS; i++) {
		if (w->chunks[i].loaded)
			save_chunk(w, &w->chunks[i]);
	}
}

SDL_bool is_chunk_saved(world *w, SDL_Point chunk_pos) {
	filename name;
	get_chunk_filename(chunk_pos, name);

	SDL_RWops *file = SDL_RWFromFile(name, "a+b");
	sdl_error_assert(file);
	Sint64 len = SDL_RWseek(file, 0, RW_SEEK_END);
	SDL_RWclose(file);

	return len != 0;
}

SDL_bool load_saved_chunk(world *w, chunk *c) {
	if (!is_chunk_saved(w, c->pos)) return SDL_FALSE;

	filename name;
	get_chunk_filename(c->pos, name);

	SDL_RWops *file = SDL_RWFromFile(name, "rb");
	read_header(file);

	sdl_error_assert(
		SDL_RWread(file, c->tiles, sizeof(c->tiles), 1));

	SDL_RWclose(file);
	return SDL_TRUE;
}

SDL_bool load_world(world *w) {
	filename name;
	get_world_filename(name);

	SDL_RWops *file = SDL_RWFromFile(name, "a+b");
	sdl_error_assert(file);
	if (SDL_RWseek(file, 0, RW_SEEK_END) == 0) {
		SDL_RWclose(file);
		return SDL_FALSE;
	}
	SDL_RWseek(file, 0, RW_SEEK_SET);
	read_header(file);
	
	sdl_error_assert(
		SDL_RWread(file, &w->player.pos, sizeof(w->player.pos), 1));
	sdl_error_assert(
		SDL_RWread(file, &w->player.scores, sizeof(w->player.scores), 1));

	SDL_RWclose(file);
	return SDL_TRUE;
}
