#include <SDL.h>

#include "digits.h"

static SDL_Texture *get_font(SDL_Renderer *rend) {
	static SDL_Texture *font = NULL;
	if (!font) {
		font = SDL_CreateTexture(rend,
			SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
			FONT_WIDTH, sizeof(digits) / FONT_WIDTH);
		SDL_SetTextureBlendMode(font, SDL_BLENDMODE_BLEND);
		SDL_assert(font);

		int pitch;
		void *out;
		SDL_assert(SDL_LockTexture(font, NULL, &out, &pitch) >= 0);
		SDL_Color *pixels = out;

		for (int i = 0; i < sizeof(digits); i++) {
			pixels[i] =
				(SDL_Color) {0xFF, 0xFF, 0xFF, digits[i] ? 0xFF : 0x00};
		}

		SDL_UnlockTexture(font);
	}
	return font;
}

int draw_counter(SDL_Renderer *rend,
		SDL_Point screen_pos, double size, SDL_Color color,
		int number, int digits) {
	double scale = size / FONT_HEIGHT;

	SDL_Texture *font = get_font(rend);
	SDL_SetTextureColorMod(font, color.r, color.g, color.b);

	if (digits == 0) {
		for (int n = number; n > 0; n /= DIGIT_BASE)
			digits += 1;
		digits = SDL_max(digits, 1);
	}

	int n = number;
	for (int i = digits - 1; i >= 0; i--) {
		int digit = n % DIGIT_BASE;

		int width = SDL_round((double) FONT_WIDTH * scale);
		int height = SDL_round((double) FONT_HEIGHT * scale);
		int offset = SDL_round((double) FONT_WIDTH * scale * (double) i);
		SDL_Rect draw = {screen_pos.x + offset, screen_pos.y, width, height};
		SDL_Rect digit_rect = {
			0, FONT_HEIGHT * digit, FONT_WIDTH, FONT_HEIGHT,
		};
		SDL_RenderCopy(rend, font, &digit_rect, &draw);

		n /= DIGIT_BASE;
	}

	return SDL_round((double) FONT_WIDTH * scale * (double) digits);
}
