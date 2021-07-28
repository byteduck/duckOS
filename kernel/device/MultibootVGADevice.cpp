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

#include <kernel/kstd/defines.h>
#include "MultibootVGADevice.h"
#include <kernel/memory/PageDirectory.h>
#include <kernel/tasking/Process.h>

MultibootVGADevice *MultibootVGADevice::create(struct multiboot_info *mboot_header) {
	auto* ret = new MultibootVGADevice();
	if(!ret->detect(mboot_header)) {
		delete ret;
		return nullptr;
	}
	return ret;
}

MultibootVGADevice::~MultibootVGADevice() {
	if(double_buffer) delete[] double_buffer;
}

bool MultibootVGADevice::detect(struct multiboot_info *mboot_header) {
	if(!(mboot_header->flags & MULTIBOOT_INFO_FRAMEBUFFER_INFO)) {
		printf("[VGA] Unable to find multiboot framebuffer info!\n");
		return false;
	}

	framebuffer_paddr = mboot_header->framebuffer_addr;
	framebuffer_pitch = mboot_header->framebuffer_pitch;
	framebuffer_bpp = mboot_header->framebuffer_bpp;
	framebuffer_height = mboot_header->framebuffer_height;
	framebuffer_width = mboot_header->framebuffer_width;
	switch(mboot_header->framebuffer_type) {
		case MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT:
			textmode = true;
			break;
		case MULTIBOOT_FRAMEBUFFER_TYPE_RGB:
			textmode = false;
			break;
		default:
			printf("[VGA] Unsupported multiboot framebuffer type %d!\n", mboot_header->framebuffer_type);
			return false;
	}

	framebuffer = (uint32_t*) PageDirectory::k_mmap(framebuffer_paddr, framebuffer_size(), true);
	double_buffer = new uint32_t[framebuffer_size() / sizeof(uint32_t)];
	memset(double_buffer, 0, framebuffer_size());

	return true;
}

ssize_t MultibootVGADevice::write(FileDescriptor &fd, size_t offset, const uint8_t *buffer, size_t count) {
	LOCK(_lock);
	if(!double_buffer || !framebuffer) return -ENOSPC;
	if(offset + count > framebuffer_size()) return -ENOSPC;
	memcpy(((uint8_t*)double_buffer + offset), buffer, count);
	memcpy(((uint8_t*)framebuffer + offset), buffer, count);
	return count;
}

void MultibootVGADevice::set_pixel(size_t x, size_t y, uint32_t value) {
	if(x > framebuffer_width || y > framebuffer_height) return;
	double_buffer[x + y * framebuffer_width] = value;
	framebuffer[x + y * framebuffer_width] = value;
}

bool MultibootVGADevice::is_textmode() {
	return textmode;
}

size_t MultibootVGADevice::get_display_width() {
	return framebuffer_width;
}

size_t MultibootVGADevice::get_display_height() {
	return framebuffer_height;
}

uint32_t *MultibootVGADevice::get_framebuffer() {
	return framebuffer;
}

size_t MultibootVGADevice::framebuffer_size() {
	return framebuffer_pitch * framebuffer_height;
}

void MultibootVGADevice::scroll(size_t pixels) {
	if(pixels > framebuffer_height) return;
	memcpy(double_buffer, double_buffer + pixels * framebuffer_width, (framebuffer_height - pixels) * framebuffer_width * sizeof(uint32_t));
	memset(double_buffer + (framebuffer_height - pixels) * framebuffer_width, 0, framebuffer_width * pixels * sizeof(uint32_t));
	memcpy(framebuffer, double_buffer, framebuffer_size());
}

void MultibootVGADevice::clear(uint32_t color) {
	size_t size = framebuffer_height * framebuffer_width;
	for(size_t i = 0; i < size; i++) {
		double_buffer[i] = color;
		framebuffer[i] = color;
	}
}

void* MultibootVGADevice::map_framebuffer(const kstd::shared_ptr<Process>& proc) {
	void* ret = proc->page_directory->mmap(framebuffer_paddr, framebuffer_size(), true);
	if(!ret)
		return (void*) -ENOMEM;
	return ret;
}
