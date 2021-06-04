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
    
    Copyright (c) Byteduck 2016-2020. All rights reserved.
*/

#include <ctype.h>

const char _ctype_[256] = {
		_C, _C, _C, _C, _C, _C, _C, _C,
		_C, _C | _S, _C | _S, _C | _S, _C | _S, _C | _S, _C, _C,
		_C, _C, _C, _C, _C, _C, _C, _C,
		_C, _C, _C, _C, _C, _C, _C, _C,
		(char)(_S | _B), _P, _P, _P, _P, _P, _P, _P,
		_P, _P, _P, _P, _P, _P, _P, _P,
		_N, _N, _N, _N, _N, _N, _N, _N,
		_N, _N, _P, _P, _P, _P, _P, _P,
		_P, _U | _X, _U | _X, _U | _X, _U | _X, _U | _X, _U | _X, _U,
		_U, _U, _U, _U, _U, _U, _U, _U,
		_U, _U, _U, _U, _U, _U, _U, _U,
		_U, _U, _U, _P, _P, _P, _P, _P,
		_P, _L | _X, _L | _X, _L | _X, _L | _X, _L | _X, _L | _X, _L,
		_L, _L, _L, _L, _L, _L, _L, _L,
		_L, _L, _L, _L, _L, _L, _L, _L,
		_L, _L, _L, _P, _P, _P, _P, _C
};

#undef isalnum
int isalnum(int c) {
	return __isalnum(c);
}

#undef isalpha
int isalpha(int c) {
	return __isalpha(c);
}

#undef isblank
int isblank(int c) {
	return __isblank(c);
}

#undef iscntrl
int iscntrl(int c) {
	return __iscntrl(c);
}

#undef isdigit
int isdigit(int c) {
	return __isdigit(c);
}

#undef isgraph
int isgraph(int c) {
	return __isgraph(c);
}

#undef islower
int islower(int c) {
	return __islower(c);
}

#undef isprint
int isprint(int c) {
	return __isprint(c);
}

#undef ispunct
int ispunct(int c) {
	return __ispunct(c);
}

#undef isspace
int isspace(int c) {
	return __isspace(c);
}

#undef isupper
int isupper(int c) {
	return __isupper(c);
}

#undef isxdigit
int isxdigit(int c) {
	return __isxdigit(c);
}

#undef tolower
int tolower(int c) {
	return __tolower(c);
}

#undef toupper
int toupper(int c) {
	return __toupper(c);
}