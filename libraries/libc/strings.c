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

#include "strings.h"
#include "ctype.h"

int strcasecmp(const char* s1, const char* s2) {
	while(tolower(*s1) == tolower(*s2)) {
		if(tolower(*s1) == 0)
			return 0;
		s1++;
		s2++;
	}
	return tolower(*((const uint8_t*)s1 - 1)) - tolower(*((const uint8_t*)(s2 - 1)));
}

int strncasecmp(const char* s1, const char* s2, size_t n) {
	if(!n) return 0;
	do {
		if(tolower(*s1) != tolower(*s2))
			return tolower(*s1) - tolower(*s2);
		if(*s1 == 0)
			return 0;
		s1++;
		s2++;
	} while(--n);
	return 0;
}