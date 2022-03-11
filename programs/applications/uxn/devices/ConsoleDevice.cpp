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
    
    Copyright (c) Byteduck 2016-2022. All rights reserved.
*/
#include <cstdio>
#include "ConsoleDevice.h"

uint16_t ConsoleDevice::read(uint8_t addr, bool is_short) {
	uint16_t ret = 0;
	if(addr == 0x2)
		fread(&ret, 1, 1, stdin);
	return ret;
}

void ConsoleDevice::write(uint8_t addr, uint16_t val, bool is_short) {
	if(addr == 0x8)
		fwrite(&val, 1, 1, stdout);
}
