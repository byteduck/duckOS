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

#include "Display.h"
#include <unistd.h>
#include <cstdio>
#include <sys/ioctl.h>
#include <kernel/device/VGADevice.h>

Display::Display() {
	framebuffer_fd = open("/dev/fb0", O_RDWR);
	if(framebuffer_fd < -1) {
		perror("Failed to open framebuffer");
		return;
	}

	if(ioctl(framebuffer_fd, IO_VIDEO_WIDTH, &_width) < 0) {
		perror("Failed to get framebuffer width");
		return;
	}

	if(ioctl(framebuffer_fd, IO_VIDEO_HEIGHT, &_height) < 0) {
		perror("Failed to get framebuffer height");
		return;
	}

	if(ioctl(framebuffer_fd, IO_VIDEO_MAP, &_framebuffer) < 0) {
		perror("Failed to map framebuffer");
		return;
	}

	printf("Framebuffer opened and mapped (%d x %d).\n", _width, _height);
}

int Display::width() {
	return _width;
}

int Display::height() {
	return _height;
}

Pixel* Display::framebuffer() {
	return _framebuffer;
}

void Display::clear(Pixel color) {
	size_t framebuffer_size = _width * _height;
	for(size_t i = 0; i < framebuffer_size; i++) {
		_framebuffer[i] = color;
	}
}
