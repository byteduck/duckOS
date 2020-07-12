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

#ifndef DUCKOS_TASKYIELD_HPP
#define DUCKOS_TASKYIELD_HPP
#include <kernel/kstdio.h>

class TaskYield {
public:
	TaskYield() = default;
	~TaskYield() = default;

	inline void set_waiting() { _ready = false; }
	inline void set_ready() { _ready = true; }
	inline bool ready() { return _ready; };
private:
	volatile bool _ready = true;
};


#endif //DUCKOS_TASKYIELD_HPP
