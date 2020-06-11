#ifndef DUCKOS_RANDOM_H
#define DUCKOS_RANDOM_H

#include <common/cstddef.h>

int rand();
void srand(unsigned int seed);
void get_random_bytes(uint8_t* buffer, size_t count);

#endif //DUCKOS_RANDOM_H
