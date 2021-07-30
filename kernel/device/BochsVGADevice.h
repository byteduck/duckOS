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

#ifndef DUCKOS_BOCHSVGADEVICE_H
#define DUCKOS_BOCHSVGADEVICE_H

#include "VGADevice.h"
#include <kernel/pci/PCI.h>
#include <kernel/tasking/SpinLock.h>

#define VBE_DISPI_INDEX_ID 0
#define VBE_DISPI_INDEX_XRES 1
#define VBE_DISPI_INDEX_YRES 2
#define VBE_DISPI_INDEX_BPP 3
#define VBE_DISPI_INDEX_ENABLE 4
#define VBE_DISPI_INDEX_BANK 5
#define VBE_DISPI_INDEX_VIRT_WIDTH 6
#define VBE_DISPI_INDEX_VIRT_HEIGHT 7
#define VBE_DISPI_INDEX_X_OFFSET 8
#define VBE_DISPI_INDEX_Y_OFFSET 9

#define VBE_DISPI_BPP_4  0x04
#define VBE_DISPI_BPP_8  0x08
#define VBE_DISPI_BPP_15 0x0F
#define VBE_DISPI_BPP_16 0x10
#define VBE_DISPI_BPP_24 0x18
#define VBE_DISPI_BPP_32 0x20

#define VBE_DISPI_DISABLED 0x0u
#define VBE_DISPI_ENABLED 0x1u
#define VBE_DISPI_LFB_ENABLED 0x40u

#define VBE_DISPI_IOPORT_INDEX 0x01CE
#define VBE_DISPI_IOPORT_DATA 0x01CF

#define VBE_DEFAULT_WIDTH 640
#define VBE_DEFAULT_HEIGHT 480

class Process;
class BochsVGADevice: public VGADevice {
public:
	static BochsVGADevice* create();

	ssize_t write(FileDescriptor& fd, size_t offset, const uint8_t* buffer, size_t count) override;
	void set_pixel(size_t x, size_t y, uint32_t value) override;

	uint32_t* get_framebuffer();
	size_t get_display_width() override;
	size_t get_display_height() override;
	void scroll(size_t pixels) override;
	void clear(uint32_t color) override;
	void* map_framebuffer(Process* proc) override;

private:
	BochsVGADevice();
	bool detect();

	void write_register(uint16_t index, uint16_t value);
	uint16_t read_register(uint16_t index);
	size_t framebuffer_size();
	bool set_resolution(uint16_t width, uint16_t height);

	virtual int ioctl(unsigned request, void* argp) override;

	PCI::Address address = {0,0,0};
	uint32_t framebuffer_paddr = 0;
	uint32_t* framebuffer = nullptr;
	uint16_t display_width = VBE_DEFAULT_WIDTH;
	uint16_t display_height = VBE_DEFAULT_HEIGHT;

	SpinLock _lock;
};


#endif //DUCKOS_BOCHSVGADEVICE_H
