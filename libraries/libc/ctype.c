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