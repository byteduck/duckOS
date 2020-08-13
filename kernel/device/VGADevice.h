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

#ifndef DUCKOS_VGADEVICE_H
#define DUCKOS_VGADEVICE_H


#include "BlockDevice.h"


class VGADevice: public BlockDevice {
public:
	VGADevice();
	static VGADevice& inst() {return *_inst;};
	virtual void scroll(size_t pixels) = 0;
	virtual void set_pixel(size_t x, size_t y, uint32_t value) = 0;
	virtual size_t get_display_width() = 0;
	virtual size_t get_display_height() = 0;
	virtual void clear(uint32_t color) = 0;
private:
	static VGADevice* _inst;
};


#endif //DUCKOS_VGADEVICE_H
