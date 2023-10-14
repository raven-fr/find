#include <SDL.h>

static Uint64 random = 0xbee;

Uint64 hash(Uint64 n) {
	Uint64 h = 0;
	for (int i = 0; i < 8; i++) {
		h += (n >> i * 8) & 0xFF;
		h += h << 10;
		h ^= h >> 6;
	}
	h += h << 3;
	h ^= h >> 11;
	h += h << 15;
	return h;
}

void seed_rand(Uint64 seed) {
	random ^= seed;
}

Uint64 get_rand() {
	random = hash(random);
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
