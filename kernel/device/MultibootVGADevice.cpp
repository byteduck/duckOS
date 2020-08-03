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

#include <common/defines.h>
#include "MultibootVGADevice.h"
#include <kernel/memory/PageDirectory.h>

MultibootVGADevice *MultibootVGADevice::create(struct multiboot_info *mboot_header) {
	auto* ret = new MultibootVGADevice();
	if(!ret->detect(mboot_header)) {
		delete ret;
		return nullptr;
	}
	return ret;
}

MultibootVGADevice::MultibootVGADevice(): BlockDevice(29, 0) {}

bool MultibootVGADevice::detect(struct multiboot_info *mboot_header) {
	if(!(mboot_header->flags & MULTIBOOT_INFO_FRAMEBUFFER_INFO)) {
		printf("vga: Unable to find multiboot framebuffer info!\n");
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
			printf("vga: Unsupported multiboot framebuffer type %d!\n", mboot_header->framebuffer_type);
			return false;
	}

	framebuffer = (uint32_t*) PageDirectory::k_mmap(framebuffer_paddr, framebuffer_size(), true);

	return true;
}

ssize_t MultibootVGADevice::write(FileDescriptor &fd, size_t offset, const uint8_t *buffer, size_t count) {
	LOCK(_lock);
	if(!framebuffer) return -ENOSPC;
	if(offset + count > framebuffer_size()) return -ENOSPC;
	memcpy(((uint8_t*)framebuffer + offset), buffer, count);
	return count;
}

bool MultibootVGADevice::is_textmode() {
	return textmode;
}

uint32_t MultibootVGADevice::get_framebuffer_width() {
	return framebuffer_width;
}

uint32_t MultibootVGADevice::get_framebuffer_height() {
	return framebuffer_height;
}

uint32_t *MultibootVGADevice::get_framebuffer() {
	return framebuffer;
}

size_t MultibootVGADevice::framebuffer_size() {
	return framebuffer_pitch * framebuffer_height;
}
