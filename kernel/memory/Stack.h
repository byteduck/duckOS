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

#include <kernel/kstd/cstring.h>

class Stack {
public:
	Stack(void* stackptr, size_t real_stackptr):
		_stackptr((size_t) stackptr),
		_real_stackptr(real_stackptr) {}

	Stack(void* stackptr): Stack(stackptr, (size_t) stackptr) {}

	Stack(const Stack& other):
		_stackptr(other._stackptr),
		_real_stackptr(other._real_stackptr) {}

	inline Stack& operator=(const Stack& other) noexcept {
		_stackptr = other._stackptr;
		_real_stackptr = other._real_stackptr;
		return *this;
	}

	template<typename T>
	inline void push(T val) {
		_stackptr -= sizeof(val);
		_real_stackptr -= sizeof(T);
		*((T*) _stackptr) = val;
	}

	inline void push_string(const char* str) {
		size_t len = strlen(str) + 1;
		_stackptr -= len;
		_real_stackptr -= sizeof(char) * len;
		strcpy((char*)_stackptr, str);
	}

	inline size_t real_stackptr() const {
		return _real_stackptr;
	}

	inline void* stackptr() const {
		return (void*) _stackptr;
	}

private:
	size_t _real_stackptr;
	size_t _stackptr;
};

