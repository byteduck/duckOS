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

#include "TTYDevice.h"
#include <libterm/Terminal.h>
#include <kernel/device/KeyboardDevice.h>

class VirtualTTY: public TTYDevice, private KeyboardHandler, private Term::Listener {
public:
	VirtualTTY(unsigned int major, unsigned int minor);
	static kstd::Arc<VirtualTTY> current_tty();

	void set_active();
	bool active();
	void clear();
	void set_graphical(bool graphical);
	Term::Terminal* get_terminal();

protected:
	static void register_tty(size_t id, VirtualTTY* device);
	static void set_current_tty(size_t tty);

private:
	static size_t _current_tty;
	static kstd::Arc<VirtualTTY> _ttys[NUM_TTYS];
	size_t _id;
	Term::Terminal* terminal = nullptr;

	//File
	int ioctl(unsigned request, SafePointer<void*> argp) override;

	//KeyboardHandler
	void handle_key(KeyEvent) override;

	//TTYDevice
	size_t tty_write(const uint8_t* buffer, size_t count) override;
	void echo(uint8_t c) override;

	//Terminal::Listener
	void on_character_change(const Term::Position& position, const Term::Character& character) override;
	void on_cursor_change(const Term::Position& position) override;
	void on_backspace(const Term::Position& position) override;
	void on_clear() override;
	void on_clear_line(int line) override;
	void on_scroll(int lines) override;
	void on_resize(const Term::Size& old_size, const Term::Size& new_size) override;
	void emit(const uint8_t* data, size_t size) override;

	bool _active = false;
	bool _graphical = false;
};


