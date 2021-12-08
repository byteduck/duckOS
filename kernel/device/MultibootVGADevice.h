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

#pragma once

#include "VGADevice.h"
#include <kernel/tasking/SpinLock.h>

class MultibootVGADevice: VGADevice {
public:
	static MultibootVGADevice* create(struct multiboot_info* mboot_header);
	~MultibootVGADevice();

	ssize_t write(FileDescriptor& fd, size_t offset, const uint8_t* buffer, size_t count) override;
	void set_pixel(size_t x, size_t y, uint32_t value) override;

	bool is_textmode();
	size_t get_display_width() override;
	size_t get_display_height() override;
	uint32_t* get_framebuffer();
	size_t framebuffer_size();
	void scroll(size_t pixels) override;
	void clear(uint32_t color) override;
	void* map_framebuffer(Process* proc) override;

private:
	MultibootVGADevice() = default;
	bool detect(struct multiboot_info* mboot_header);
	size_t framebuffer_paddr;
	uint32_t* framebuffer;
	uint32_t* double_buffer = nullptr;
	uint32_t framebuffer_pitch;
	uint32_t framebuffer_width;
	uint32_t framebuffer_height;
	uint32_t framebuffer_bpp;
	bool textmode;
	SpinLock _lock;
};


