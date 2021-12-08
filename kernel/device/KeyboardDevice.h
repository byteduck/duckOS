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

#pragma once

#include "CharacterDevice.h"
#include <kernel/keyboard.h>
#include <kernel/kstd/circular_queue.hpp>
#include <kernel/interrupt/IRQHandler.h>

class KeyEvent {
public:
	uint16_t scancode;
	uint8_t key;
	uint8_t character;
	uint8_t modifiers;
	bool pressed() const;
};

class KeyboardHandler {
public:
	virtual void handle_key(KeyEvent) = 0;
};

class KeyboardDevice: public CharacterDevice, public IRQHandler {
public:
	static KeyboardDevice* inst();

	KeyboardDevice();

	//File
	ssize_t read(FileDescriptor& fd, size_t offset, uint8_t* buffer, size_t count) override;
	ssize_t write(FileDescriptor& fd, size_t offset, const uint8_t* buffer, size_t count) override;
	bool can_read(const FileDescriptor& fd) override;
	bool can_write(const FileDescriptor& fd) override;

	//IRQHandler
	void set_handler(KeyboardHandler* handler);
	void handle_irq(Registers* regs) override;

	//I8042
	void handle_byte(uint8_t byte);

private:
	static KeyboardDevice* _instance;

	void set_mod(uint8_t mod, bool state);
	void set_key_state(uint8_t key, bool pressed);

	kstd::circular_queue<KeyEvent> _event_buffer;
	KeyboardHandler* _handler = nullptr;
	uint8_t _modifiers = 0;
	bool _e0_flag = false;
	SpinLock _lock;
};


