#include <SDL.h>

static Uint64 random = 0xbee;

void seed_rand(Uint64 seed) {
	random ^= seed;
}

Uint64 get_rand() {
	random ^= random >> 7;
	random ^= random << 9;
	random ^= random >> 13;
	return random;
}

int rand_int() {
	Uint64 rand = get_rand();
	int result;
	memcpy(&result, &rand, sizeof(int));
	result = SDL_abs(result);
	return result;
}

double rand_float() {
	return (get_rand() >> 11) * 0x1.0p-53;
}
