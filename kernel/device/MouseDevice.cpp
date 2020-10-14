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

#include <kernel/random.h>
#include "MouseDevice.h"

MouseDevice* MouseDevice::instance;

MouseDevice *MouseDevice::inst() {
	return instance;
}

MouseDevice::MouseDevice(): CharacterDevice(13, 1), event_buffer(128)  {
	instance = this;
	printf("[device] Initializing mouse...\n");

	//Wait for mouse to be ready
	while((inb(I8042_STATUS) & 1u)) {
		inb(I8042_BUFFER);
	}

	//Enable auxiliary device port
	wait_write();
	outb(I8042_STATUS, 0xA8);

	//Check for presence of mouse
	write(MOUSE_REQUEST_SINGLE_PACKET);
	if(read() == I8042_ACK) {
		present = true;
		read();
		read();
		read(); //Flush buffer
	} else {
		present = false;
		printf("[device] Mouse not present!\n");
		return;
	}

	io_wait();

	//Enable IRQs
	wait_write();
	outb(I8042_STATUS, 0x20);
	wait_write();
	uint8_t status = inb(I8042_BUFFER) | 2u;
	wait_write();
	outb(I8042_STATUS, 0x60);
	wait_write();
	outb(I8042_BUFFER, status);

	//Set defaults and enable
	write(MOUSE_SET_DEFAULTS);
	read();
	write(MOUSE_ENABLE_PACKET_STREAMING);
	read();

	//Get the device ID
	write(MOUSE_GET_DEVICE_ID);
	read(); //ACK
	uint8_t id = read();

	if(id != MOUSE_INTELLIMOUSE_ID) {
		//Initialize the scroll wheel
		write(MOUSE_SET_SAMPLE_RATE);
		read(); //ACK
		write(200);
		read(); //ACK
		write(MOUSE_SET_SAMPLE_RATE);
		read(); //ACK
		write(100);
		read(); //ACK
		write(MOUSE_SET_SAMPLE_RATE);
		read(); //ACK
		write(80);
		read(); //ACK

		write(MOUSE_GET_DEVICE_ID);
		read(); //ACK
		id = read();
	}

	if(id == MOUSE_INTELLIMOUSE_ID) {
		has_scroll_wheel = true;
		printf("[device] Mouse has wheel.\n");
	}

	set_irq(12);
	reinstall_irq();

	printf("[device] Mouse initialized!\n");
}

ssize_t MouseDevice::read(FileDescriptor &fd, size_t offset, uint8_t *buffer, size_t count) {
	LOCK(lock);
	size_t ret = 0;
	while(ret < count) {
		if(event_buffer.empty()) break;
		if((count - ret) < sizeof(MouseEvent)) break;
		auto evt = event_buffer.pop_front();
		memcpy(buffer, &evt, sizeof(MouseEvent));
		ret += sizeof(MouseEvent);
		buffer += sizeof(MouseEvent);
	}
	return ret;
}

ssize_t MouseDevice::write(FileDescriptor &fd, size_t offset, const uint8_t *buffer, size_t count) {
	return 0;
}

void MouseDevice::handle_irq(Registers *regs) {
	while(true) {
		uint8_t status = inb(I8042_STATUS);
		if (!(((status & I8042_WHICH_BUFFER) == I8042_MOUSE_BUFFER) && (status & I8042_BUFFER_FULL)))
			return;

		uint8_t data = inb(I8042_BUFFER);
		packet_data[packet_state] = data;
		switch(packet_state) {
			case 0:
				if(!(data & 0x8u)) break;
				packet_state++;
				break;
			case 1:
				packet_state++;
				break;
			case 2:
				if(has_scroll_wheel) {
					packet_state++;
					break;
				}
				handle_packet();
			case 3:
				handle_packet();
				break;
		}
	}
}

void MouseDevice::wait_read() {
	uint32_t timeout = 100000;
	while(--timeout) {
		if(inb(I8042_STATUS) & 0x1u)
			return;
	}

	printf("Mouse timed out (read)\n");
}

void MouseDevice::wait_write() {
	uint32_t timeout = 100000;
	while(--timeout) {
		if(!(inb(I8042_STATUS) & 0x2u))
			return;
	}
	printf("Mouse timed out (write)\n");
}

uint8_t MouseDevice::read() {
	wait_read();
	return inb(I8042_BUFFER);
}

void MouseDevice::write(uint8_t value) {
	wait_write();
	outb(I8042_STATUS, 0xD4);
	wait_write();
	outb(I8042_BUFFER, value);
}

bool MouseDevice::can_read(const FileDescriptor& fd) {
	return !event_buffer.empty();
}

bool MouseDevice::can_write(const FileDescriptor& fd) {
	return false;
}

void MouseDevice::handle_packet() {
	packet_state = 0;
	int x = packet_data[1];
	int y = packet_data[2];
	int z = 0;

	//Handle scrolling if applicable
	if(has_scroll_wheel && packet_data[3]) {
		z = (char) (packet_data[3] & 0xfu);
		if(z == 15)
			z = -1;
	}

	if(x && (packet_data[0] & 0x10u))
		x -= 0x100; //Sign is negative
	if(y && (packet_data[0] & 0x20u))
		y -= 0x100; //Sign is negative
	if(packet_data[0] & 0xc0u) {
		//There was overflow
		x = 0;
		y = 0;
	}

	event_buffer.push({x, y, z, (uint8_t) (packet_data[0] & 0x7u)});
}
