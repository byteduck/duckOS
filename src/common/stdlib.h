#ifndef DUCKOS_STDLIB_H
#define DUCKOS_STDLIB_H

#include <common/cstddef.h>
#include <common/cstring.h>

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

int atoi(char *str);

int sgn(int x);

int abs(float x);

char nibbleToHexString(uint8_t num);

char *itoa(int i, char *p, int base);

#endif //DUCKOS_STDLIB_H
