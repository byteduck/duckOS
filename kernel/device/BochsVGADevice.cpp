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
#include <kernel/pci/pci.h>
#include <common/defines.h>

BochsVGADevice* BochsVGADevice::instance;
PCI::ID bochs_qemu_vga = {0x1234, 0x1111};
PCI::ID vbox_vga = {0x80ee, 0xbeef};

BochsVGADevice::BochsVGADevice(): BlockDevice(29, 0) {
	instance = this;

	PCI::enumerate_devices([](PCI::Address address, PCI::ID id, void* dataPtr) {
		if(id == bochs_qemu_vga || id == vbox_vga) {
			*((PCI::Address*)dataPtr) = address;
		}
	}, &address);

	if(address.bus == 0 && address.function == 0 && address.slot == 0) printf("vga: Could not find a bochs-compatible VGA device!\n");
	else {
		framebuffer_paddr = PCI::read_dword(address, PCI_BAR0) & 0xfffffff0;
		set_resolution(VBE_DEFAULT_WIDTH, VBE_DEFAULT_HEIGHT);
		framebuffer = (uint32_t*)Paging::PageDirectory::k_mmap(framebuffer_paddr, framebuffer_size(), true);
		set_graphical_mode(true);
		printf("vga: Found a bochs-compatible VGA device at %x:%x.%x\n", address.bus, address.slot, address.function);
		printf("vga: virtual framebuffer mapped from 0x%x to 0x%x\n", framebuffer_paddr, framebuffer);
	}
}

BochsVGADevice& BochsVGADevice::inst() {
	return *instance;
}

void BochsVGADevice::write_register(uint16_t index, uint16_t value) {
	outw(VBE_DISPI_IOPORT_INDEX, index);
	outw(VBE_DISPI_IOPORT_DATA, value);
}

uint16_t BochsVGADevice::read_register(uint16_t index) {
	outw(VBE_DISPI_IOPORT_INDEX, index);
	return inw(VBE_DISPI_IOPORT_DATA);
}

bool BochsVGADevice::set_resolution(uint16_t width, uint16_t height) {
	//Write registers
	write_register(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
	write_register(VBE_DISPI_INDEX_XRES, width);
	write_register(VBE_DISPI_INDEX_YRES, height);
	write_register(VBE_DISPI_INDEX_VIRT_WIDTH, width);
	write_register(VBE_DISPI_INDEX_VIRT_HEIGHT, height);
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
	if(!framebuffer) return -ENOSPC;
	if(offset + count > framebuffer_size()) return -ENOSPC;
	memcpy(((uint8_t*)framebuffer + offset), buffer, count);
	return count;
}

uint32_t *BochsVGADevice::get_framebuffer() {
	return framebuffer;
}

uint16_t BochsVGADevice::get_framebuffer_width() {
	return display_width;
}

uint16_t BochsVGADevice::get_framebuffer_height() {
	return display_height;
}
