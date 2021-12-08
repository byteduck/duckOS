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

#ifndef DUCKOS_KERNEL_ATOMIC_H
#define DUCKOS_KERNEL_ATOMIC_H


class Atomic {
public:
	static inline int swap(volatile int* const var, int value) {
		asm("xchg %0, %1" : "=r"(value), "=m"(*var) : "0"(value) : "memory");
		return value;
	}

	static inline void store(volatile int* const var, int value) {
		asm("movl %1, %0" : "=m"(*var) : "r"(value) : "memory");
	}

	static inline void inc(volatile int* var) {
		asm("lock;incl %0" : "=m"(*var) : "m"(*var) : "memory");
	}

	static inline void dec(volatile int* var) {
		asm("lock;decl %0" : "=m"(*var) : "m"(*var) : "memory");
	}
};


#endif //DUCKOS_KERNEL_ATOMIC_H
