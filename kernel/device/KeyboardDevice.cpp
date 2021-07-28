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

#include <kernel/tasking/TaskManager.h>
#include <kernel/IO.h>
#include "KeyboardDevice.h"
#include "I8042.h"

//KeyEvent

bool KeyEvent::pressed() const {return !(scancode & KBD_IS_PRESSED);}

//KeyboardDevice

KeyboardDevice* KeyboardDevice::_instance;

KeyboardDevice* KeyboardDevice::inst() { return _instance;}

KeyboardDevice::KeyboardDevice(): CharacterDevice(13, 0), IRQHandler(1), _event_buffer(1024) {
	_instance = this;
}

ssize_t KeyboardDevice::read(FileDescriptor &fd, size_t offset, uint8_t *buffer, size_t count) {
	LOCK(_lock);
	size_t ret = 0;
	while(ret < count) {
		if(_event_buffer.empty())
			break;
		if((count - ret) < sizeof(KeyEvent))
			break;
		auto evt = _event_buffer.pop_front();
		memcpy(buffer, &evt, sizeof(KeyEvent));
		ret += sizeof(KeyEvent);
		buffer += sizeof(KeyEvent);
	}
	return ret;
}

ssize_t KeyboardDevice::write(FileDescriptor &fd, size_t offset, const uint8_t *buffer, size_t count) {
	return 0;
}

bool KeyboardDevice::can_read(const FileDescriptor& fd) {
	return !_event_buffer.empty();
}

bool KeyboardDevice::can_write(const FileDescriptor& fd) {
	return false;
}

void KeyboardDevice::set_handler(KeyboardHandler *handler) {
	_handler = handler;
}

void KeyboardDevice::handle_irq(Registers *regs) {
	I8042::inst().handle_irq();
}

void KeyboardDevice::handle_byte(uint8_t byte) {
	auto scancode = byte;
	auto key = scancode & 0x7fu;
	bool key_pressed = !(scancode & KBD_IS_PRESSED);

	if(scancode == 0xE0)
		_e0_flag = true;

	//Check for modifier keys
	switch(key) {
		case KBD_SCANCODE_ALT:
			if(_e0_flag)
				set_mod(KBD_MOD_ALTGR, key_pressed);
			else
				set_mod(KBD_MOD_ALT, key_pressed);
			break;
		case KBD_SCANCODE_LSHIFT:
		case KBD_SCANCODE_RSHIFT:
			set_mod(KBD_MOD_SHIFT, key_pressed);
			break;
		case KBD_SCANCODE_CTRL:
			set_mod(KBD_MOD_CTRL, key_pressed);
			break;
		case KBD_SCANCODE_SUPER:
			set_mod(KBD_MOD_SUPER, key_pressed);
			break;
		case KBD_ACK:
		default:
			break;
	}

	set_key_state(scancode, key_pressed);
	TaskManager::yield_if_idle();
}

void KeyboardDevice::set_mod(uint8_t mod, bool state) {
	if(state)
		_modifiers |= mod;
	if(!state)
		_modifiers &= ~mod;
}

//TODO: Numpad, e0 prefixed characters, caps lock
void KeyboardDevice::set_key_state(uint8_t scancode, bool pressed) {
	uint8_t key = scancode & 0x7fu;
	char character = (_modifiers & KBD_MOD_SHIFT) ? kbd_us_shift_map[key] : kbd_us_map[key];
	KeyEvent event = {
			.scancode = (uint16_t) (_e0_flag ? 0xe000u + scancode : scancode),
			.key = key,
			.character = (uint8_t) character,
			.modifiers = _modifiers
	};
	if (_handler != nullptr)
		_handler->handle_key(event);
	_e0_flag = false;
	LOCK(_lock);
	if(!_event_buffer.push_back(event))
		printf("[I8042/Keyboard] Event buffer full!\n");
}