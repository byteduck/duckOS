#ifndef DUCKOS_STDLIB_H
#define DUCKOS_STDLIB_H

#include <common/cstddef.h>
#include <common/cstring.h>

int atoi(char *str);

int sgn(int x);

int abs(float x);

char nibbleToHexString(uint8_t num);

char *itoa(int i, char *p, int base);

#endif //DUCKOS_STDLIB_H
