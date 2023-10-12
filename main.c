#include <SDL.h>

#include "world.h"
#include "random.h"
#include "save.h"

static SDL_Window *win;
static SDL_Renderer *rend;

static world game = {0};

static int run() {
	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		seed_rand(e.common.timestamp);

		switch (e.type) {
		case SDL_QUIT:
			save_world(&game);
			return 0;
		case SDL_KEYDOWN:
			break;
		case SDL_KEYUP:
			break;
		default:
			break;
		}
	}

	int nkeys;
	const Uint8 *k = SDL_GetKeyboardState(&nkeys);

	if (k[SDL_SCANCODE_W])
		player_walk(&game, 0, -1);
	if (k[SDL_SCANCODE_S])
		player_walk(&game, 0, 1);
	if (k[SDL_SCANCODE_A])
		player_walk(&game, -1, 0);
	if (k[SDL_SCANCODE_D])
		player_walk(&game, 1, 0);

	if (!k[SDL_SCANCODE_LSHIFT]) {
		if (k[SDL_SCANCODE_UP])
			player_place(&game, 0, -1);
		if (k[SDL_SCANCODE_DOWN])
			player_place(&game, 0, 1);
		if (k[SDL_SCANCODE_LEFT])
			player_place(&game, -1, 0);
		if (k[SDL_SCANCODE_RIGHT])
			player_place(&game, 1, 0);
	} else {
		if (k[SDL_SCANCODE_UP])
			player_destroy(&game, 0, -1);
		if (k[SDL_SCANCODE_DOWN])
			player_destroy(&game, 0, 1);
		if (k[SDL_SCANCODE_LEFT])
			player_destroy(&game, -1, 0);
		if (k[SDL_SCANCODE_RIGHT])
			player_destroy(&game, 1, 0);
	}

	tick_world(&game);
	draw_world(&game, win, rend);
	SDL_RenderPresent(rend);
	SDL_Delay(1);
	return 1;
}

int main(int argc, char *argv[]) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("error initializing SDL: %s", SDL_GetError());
		return 1;
	}
	win = SDL_CreateWindow("find",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 960, 720,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (!win) {
		SDL_Log("error creating window: %s", SDL_GetError());
		return 1;
	}
	rend = SDL_CreateRenderer(win, -1, SDL_RENDERER_PRESENTVSYNC);
	if (!rend) {
		SDL_Log("error creating renderer: %s", SDL_GetError());
		return 1;
	}

	init_world(&game);
	while (run());

	SDL_DestroyRenderer(rend);
	SDL_DestroyWindow(win);
	return 0;
}
