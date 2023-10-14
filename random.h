#ifndef RANDOM_H
#define RANDOM_H

Uint64 hash(Uint64 i);
void seed_rand(Uint64 seed);
Uint64 get_rand(void);
int rand_int(void);
double rand_float(void);

#endif
