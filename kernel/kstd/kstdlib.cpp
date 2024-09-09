/*
	This file is part of duckOS.

	duckOS is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	duckOS is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with duckOS.  If not, see <https://www.gnu.org/licenses/>.

	Copyright (c) Byteduck 2016-2021. All rights reserved.
*/

#include "kstdlib.h"
#include "cstring.h"

int atoi(char *str){
	int len = strlen(str);
	int ret = 0;
	for(int i = 0; i < len; i++){
		ret = ret * 10 + (str[i] - '0');
	}
	return ret;
}

int sgn(int x){
	if(x>0) return 1;
	else return -1;
	return 0;
}

int abs(float x){
	return (int)x;
}

char nibble_to_hex(uint8_t num){
	uint8_t tmp = num & 0xF;
	if(tmp < 0xA){
		return tmp+0x30;
	}else{
		return tmp+0x57;
	}
}

uint8_t parse_hex_char(char c) {
	if(c >= '0' && c <= '9') {
		return c - '0';
	} else if(c >= 'a' && c <= 'f') {
		return 10 + c - 'a';
	} else if(c >= 'A' && c <= 'f') {
		return 10 + c - 'A';
	}
	return 0;
}

char* itoa(int i, char* p, int base) {
	if (base != 10)
		return lltoa((long long) (i & ~0x0u), p, base);
	else
		return lltoa(i, p, base);
}



extern void serial_putch(char);

char* ltoa(long i, char* p, int base) {
	if (base != 10)
		return lltoa((long long) (i & ~0x0ul), p, base);
	else
		return lltoa(i, p, base);
}

char *lltoa(long long i, char *p, int base) {
	char const digit[] = "0123456789";
	int nbcount = 0;
	bool flag = 0;
	int ind;
	if (i & 0xfffffff00000000ULL) {
		serial_putch('!');
	}
	switch(base){
		case 10: {
			bool neg = i < 0;
			if (neg) {
				p[0] = '-';
				p++;
				i *= -1;
			}
			int shifter = i;
			do {
				++p;
				shifter = shifter / 10;
			} while (shifter);
			*p = '\0';
			do {
				*--p = digit[i % 10];
				i = i / 10;
			} while (i);
			if(neg) p--;
		}
			break;
			//I figured out how to roll base 2 and 16 into one thing... Not sure how efficient it is though
		case 2:
		case 16:
			if(i == 0){
				p[0] = '0'; p[1] = '\0';
			}else{
				uint8_t shift = base == 16 ? 4 : 1;
				constexpr auto hexmask = (unsigned long long) 0xF << (sizeof(long long) * 8 - 8);
				constexpr auto binmask = (unsigned long long) 0x8 << (sizeof(long long) * 8 - 8);
				auto mask = (base == 16 ? hexmask : binmask);
				for(unsigned long long a = mask; a > 0; a = a >> shift) {
					if (((unsigned long long) i & a) != 0 || flag) {
						nbcount++;
						flag = true;
					}
				}
				ind = nbcount;
				while(ind--) {
					p[-ind + nbcount - 1] =
							base == 16 ? (nibble_to_hex((i >> (ind * 4)) & 0xF)) : (((i >> ind) & 0x1) ? '1' : '0');
				}
				p[nbcount] = '\0';
			}
			break;
	}
	return p;
}

void to_upper(char *str){
	while(*str != '\0'){
		if(*str >= 'a' && *str <= 'z') *str = *str - ('a' - 'A');
		str++;
	}
}

