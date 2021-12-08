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

#ifndef DUCKOS_STACK_H
#define DUCKOS_STACK_H

#include "LinkedMemoryRegion.h"
#include <kernel/kstd/cstring.h>

class Stack {
public:
	Stack(void* stackptr, size_t real_stackptr):
		_stackptr(stackptr),
		_stackptr8(reinterpret_cast<uint8_t*&>(_stackptr)),
		_stackptr16(reinterpret_cast<uint16_t*&>(_stackptr)),
		_stackptr32(reinterpret_cast<uint32_t*&>(_stackptr)),
		_stackptrsizet(reinterpret_cast<size_t*&>(_stackptr)),
		_stackptrint(reinterpret_cast<int*&>(_stackptr)),
		_real_stackptr(real_stackptr) {}

	Stack(void* stackptr): Stack(stackptr, (size_t) stackptr) {}

	Stack(const Stack& other):
		_stackptr(other._stackptr),
		_stackptr8(reinterpret_cast<uint8_t*&>(_stackptr)),
		_stackptr16(reinterpret_cast<uint16_t*&>(_stackptr)),
		_stackptr32(reinterpret_cast<uint32_t*&>(_stackptr)),
		_stackptrsizet(reinterpret_cast<size_t*&>(_stackptr)),
		_stackptrint(reinterpret_cast<int*&>(_stackptr)),
		_real_stackptr(other._real_stackptr) {}

	inline Stack& operator=(const Stack& other) noexcept {
		_stackptr = other._stackptr;
		_stackptr8 = reinterpret_cast<uint8_t*&>(_stackptr);
		_stackptr16 = reinterpret_cast<uint16_t*&>(_stackptr);
		_stackptr32 = reinterpret_cast<uint32_t*&>(_stackptr);
		_stackptrsizet = reinterpret_cast<size_t*&>(_stackptr);
		_stackptrint = reinterpret_cast<int*&>(_stackptr);
		_real_stackptr = other._real_stackptr;
		return *this;
	}

	inline void push8(uint8_t val) {
		*--_stackptr8 = val;
		_real_stackptr -= sizeof(uint8_t);
	}

	inline void push16(uint16_t val) {
		*--_stackptr16 = val;
		_real_stackptr -= sizeof(uint16_t);
	}

	inline void push32(uint32_t val) {
		*--_stackptr32 = val;
		_real_stackptr -= sizeof(uint32_t);
	}

	inline void push_sizet(size_t val) {
		*--_stackptrsizet = val;
		_real_stackptr -= sizeof(size_t);
	}

	inline void push_int(int val) {
		*--_stackptrint = val;
		_real_stackptr -= sizeof(int);
	}

	inline void push_string(const char* str) {
		size_t len = strlen(str) + 1;
		_stackptr8 -= len;
		_real_stackptr -= sizeof(char) * len;
		strcpy((char*)_stackptr, str);
	}

	inline size_t real_stackptr() const {
		return _real_stackptr;
	}

	inline void* stackptr() const {
		return _stackptr;
	}

private:
	size_t _real_stackptr;
	void* _stackptr;
	uint8_t*& _stackptr8;
	uint16_t*& _stackptr16;
	uint32_t*& _stackptr32;
	size_t*& _stackptrsizet;
	int*& _stackptrint;
};

#endif //DUCKOS_STACK_H
