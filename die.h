#include <SDL.h>

#include <stdlib.h>

#define die(...) (SDL_Log(__VA_ARGS__), abort())
static inline void sdl_error_assert(SDL_bool condition) {
	if (!condition) {
		SDL_Log("%s", SDL_GetError());
		// banish to hell forever
		abort();
	}
}
#define sdl_error_assert(c) (sdl_error_assert(c && SDL_TRUE))
