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

#pragma once

#include <kernel/kstd/types.h>
#include <kernel/memory/SafePointer.h>

#define RAND_MAX 65535

int rand();
void srand(unsigned int seed);
void get_random_bytes(SafePointer<uint8_t> buffer, size_t count);

template<typename T>
T rand_of() {
	static_assert(RAND_MAX == 65535);
	T ret = 0;
	for (int shift = 0; shift < sizeof(T); shift += 2)
		ret += ((T) rand()) << (shift * 8);
	return ret;
}

template<typename T>
T rand_range(T min, T max) {
	static_assert(RAND_MAX == 65535);
	return min + (rand_of<T>() % (max - min + 1));
}

