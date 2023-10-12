#include <SDL.h>

void die(void) {
	while (1) {
		// draw error message text?
		SDL_Event e;
		while (SDL_WaitEvent(&e)) SDL_Delay(1);
	}
}

void sdl_error_assert(SDL_bool condition) {
	if (!condition) {
		SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "%s", SDL_GetError());
		die();
	}
}
