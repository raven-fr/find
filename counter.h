#ifndef COUNTER_H
#define COUNTER_H

#include <SDL.h>

int draw_counter(SDL_Renderer *rend,
		SDL_Point screen_pos, int size, SDL_Color color,
		int number, int digits);

#endif
