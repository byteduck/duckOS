#include "random.h"

static unsigned long int next = 1;

int rand() {
	//FIXME: use rdrand
	next = next * 1103515245 + 12345;
	return (unsigned int)(next / 65536) % 32768;
}

void srand(unsigned int seed) {
	next = seed;
}

void get_random_bytes(uint8_t* buffer, size_t count) {
	for(auto i = 0; i < count; i++) buffer[i] = rand();
}