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

#include "BochsVGADevice.h"
#include <kernel/memory/PageDirectory.h>
#include <kernel/pci/PCI.h>
#include <kernel/kstd/defines.h>
#include <kernel/tasking/Process.h>
#include <kernel/IO.h>
#include <kernel/kstd/cstring.h>
#include <kernel/kstd/KLog.h>

PCI::ID bochs_qemu_vga = {0x1234, 0x1111};
PCI::ID vbox_vga = {0x80ee, 0xbeef};

BochsVGADevice *BochsVGADevice::create() {
	auto* ret = new BochsVGADevice();
	if(!ret->detect()) {
		delete ret;
		return nullptr;
	}
	return ret;
}

BochsVGADevice::BochsVGADevice(): VGADevice() {}

bool BochsVGADevice::detect() {
	PCI::enumerate_devices([](PCI::Address address, PCI::ID id, uint16_t type, void* dataPtr) {
		if(id == bochs_qemu_vga || id == vbox_vga) {
			*((PCI::Address*)dataPtr) = address;
		}
	}, &address);

	if(address.bus == 0 && address.function == 0 && address.slot == 0) {
		KLog::warn("VGA", "Could not find a bochs-compatible VGA device!");
		return false;
	}

	framebuffer_paddr = PCI::read_dword(address, PCI_BAR0) & 0xfffffff0;
	set_resolution(VBE_DEFAULT_WIDTH, VBE_DEFAULT_HEIGHT);
	framebuffer = (uint32_t*) PageDirectory::k_mmap(framebuffer_paddr, framebuffer_size() * 2, true);
	KLog::info("VGA", "Found a bochs-compatible VGA device at %x:%x.%x", address.bus, address.slot, address.function);
	KLog::dbg("VGA", "virtual framebuffer mapped from 0x%x to 0x%x", framebuffer_paddr, framebuffer);
	return true;
}

void BochsVGADevice::write_register(uint16_t index, uint16_t value) {
	IO::outw(VBE_DISPI_IOPORT_INDEX, index);
	IO::outw(VBE_DISPI_IOPORT_DATA, value);
}

uint16_t BochsVGADevice::read_register(uint16_t index) {
	IO::outw(VBE_DISPI_IOPORT_INDEX, index);
	return IO::inw(VBE_DISPI_IOPORT_DATA);
}

bool BochsVGADevice::set_resolution(uint16_t width, uint16_t height) {
	//Write registers
	write_register(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
	write_register(VBE_DISPI_INDEX_XRES, width);
	write_register(VBE_DISPI_INDEX_YRES, height);
	write_register(VBE_DISPI_INDEX_VIRT_WIDTH, width);
	write_register(VBE_DISPI_INDEX_VIRT_HEIGHT, height * 2);
	write_register(VBE_DISPI_INDEX_BPP, VBE_DISPI_BPP_32);
	write_register(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED);
	write_register(VBE_DISPI_INDEX_BANK, 0);

	//Test if display resolution set was successful and revert if not
	if(read_register(VBE_DISPI_INDEX_XRES) != width || read_register(VBE_DISPI_INDEX_YRES) != height) {
		set_resolution(display_width, display_height);
		return false;
	}

	display_width = width;
	display_height = height;
	return true;
}

size_t BochsVGADevice::framebuffer_size() {
	return display_height * display_width * sizeof(uint32_t);
}

ssize_t BochsVGADevice::write(FileDescriptor &fd, size_t offset, const uint8_t *buffer, size_t count) {
	LOCK(_lock);
	if(!framebuffer) return -ENOSPC;
	if(offset + count > framebuffer_size()) return -ENOSPC;
	memcpy(((uint8_t*)framebuffer + offset), buffer, count);
	return count;
}

void BochsVGADevice::set_pixel(size_t x, size_t y, uint32_t value) {
	if(x >= display_width || y >= display_height) return;
	framebuffer[x + y * display_width] = value;
}

uint32_t *BochsVGADevice::get_framebuffer() {
	return framebuffer;
}

size_t BochsVGADevice::get_display_width() {
	return display_width;
}

size_t BochsVGADevice::get_display_height() {
	return display_height;
}

void BochsVGADevice::scroll(size_t pixels) {
	if(pixels >= display_height) return;
	memcpy(framebuffer, framebuffer + pixels * display_width, (display_height - pixels) * display_width * sizeof(uint32_t));
	memset(framebuffer + (display_height - pixels) * display_width, 0, display_width * pixels * sizeof(uint32_t));
}

void BochsVGADevice::clear(uint32_t color) {
	size_t size = display_height * display_width;
	for(size_t i = 0; i < size; i++) {
		framebuffer[i] = color;
	}
}

void* BochsVGADevice::map_framebuffer(Process* proc) {
	void* ret = proc->page_directory()->mmap(framebuffer_paddr, framebuffer_size() * 2, true);
	if(!ret)
		return (void*) -ENOMEM;
	return ret;
}

int BochsVGADevice::ioctl(unsigned int request, void* argp) {
	switch(request) {
		case IO_VIDEO_OFFSET:
			if((int)argp < 0 || (int)argp > display_height)
				return -EINVAL;
			write_register(VBE_DISPI_INDEX_Y_OFFSET, (int)argp);
			return SUCCESS;
		default:
			return VGADevice::ioctl(request, argp);
	}
}
