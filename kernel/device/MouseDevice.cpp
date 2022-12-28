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

	Copyright (c) Byteduck 2016-2021. All rights reserved.
*/

#include <kernel/random.h>
#include <kernel/tasking/TaskManager.h>
#include <kernel/IO.h>
#include "MouseDevice.h"
#include "I8042.h"
#include <kernel/kstd/cstring.h>
#include <kernel/kstd/KLog.h>

MouseDevice* MouseDevice::instance;

MouseDevice *MouseDevice::inst() {
	return instance;
}

MouseDevice::MouseDevice(): CharacterDevice(13, 1), event_buffer(128), IRQHandler(12)  {
	instance = this;
	KLog::dbg("I8042/Mouse", "Initializing mouse...");
	I8042::read(I8042::MOUSE); // Drain buffer

	//Get the device ID
	I8042::write(I8042::MOUSE, MOUSE_GET_DEVICE_ID);
	I8042::read(I8042::MOUSE); //ACK
	uint8_t id = I8042::read(I8042::MOUSE);

	//Set defaults and enable
	I8042::write(I8042::MOUSE, MOUSE_SET_DEFAULTS);
	I8042::read(I8042::MOUSE);
	I8042::write(I8042::MOUSE, MOUSE_ENABLE_PACKET_STREAMING);
	I8042::read(I8042::MOUSE);

	if(id != MOUSE_INTELLIMOUSE_ID) {
		//Initialize the scroll wheel
		I8042::write(I8042::MOUSE, MOUSE_SET_SAMPLE_RATE);
		I8042::read(I8042::MOUSE); //ACK
		I8042::write(I8042::MOUSE, 200);
		I8042::read(I8042::MOUSE); //ACK
		I8042::write(I8042::MOUSE, MOUSE_SET_SAMPLE_RATE);
		I8042::read(I8042::MOUSE); //ACK
		I8042::write(I8042::MOUSE, 100);
		I8042::read(I8042::MOUSE); //ACK
		I8042::write(I8042::MOUSE, MOUSE_SET_SAMPLE_RATE);
		I8042::read(I8042::MOUSE); //ACK
		I8042::write(I8042::MOUSE, 80);
		I8042::read(I8042::MOUSE); //ACK

		I8042::write(I8042::MOUSE, MOUSE_GET_DEVICE_ID);
		I8042::read(I8042::MOUSE); //ACK
		id = I8042::read(I8042::MOUSE);
	}

	if(id == MOUSE_INTELLIMOUSE_ID) {
		has_scroll_wheel = true;
		KLog::dbg("I8042/Mouse", "Mouse has wheel.");
	}

	KLog::dbg("I8042/Mouse", "Mouse initialized!");
}

ssize_t MouseDevice::read(FileDescriptor &fd, size_t offset, SafePointer<uint8_t> buffer, size_t count) {
	size_t ret = 0;
	while(ret < count) {
		TaskManager::ScopedCritical critical;
		if(event_buffer.empty()) break;
		if((count - ret) < sizeof(MouseEvent)) break;
		auto evt = event_buffer.pop_front();
		critical.exit();
		buffer.write((uint8_t*) &evt, ret, sizeof(MouseEvent));
		ret += sizeof(MouseEvent);
	}
	return ret;
}

ssize_t MouseDevice::write(FileDescriptor &fd, size_t offset, SafePointer<uint8_t> buffer, size_t count) {
	return 0;
}

void MouseDevice::handle_irq(Registers *regs) {
	I8042::inst().handle_irq();
}

void MouseDevice::handle_byte(uint8_t byte) {
	packet_data[packet_state] = byte;
	switch(packet_state) {
		case 0:
			if(!(byte & 0x8u)) break;
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
			break;
		case 3:
			handle_packet();
			break;
	}

	TaskManager::yield_if_idle();
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

	if(event_buffer.size() == event_buffer.capacity())
		event_buffer.pop_front();
	event_buffer.push_back({x, y, z, (uint8_t) (packet_data[0] & 0x7u)});
}
