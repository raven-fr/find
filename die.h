#ifndef DIE_H
#define DIE_H

#include <SDL.h>

void die(void);
void sdl_error_assert(SDL_bool condition);

#define die(...) (SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, __VA_ARGS__), die())
#define sdl_error_assert(c) (sdl_error_assert(c && SDL_TRUE))

#endif
