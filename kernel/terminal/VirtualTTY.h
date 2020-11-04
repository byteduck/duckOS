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

#ifndef DUCKOS_VIRTUALTTY_H
#define DUCKOS_VIRTUALTTY_H

#include "TTYDevice.h"
#include <common/terminal/Terminal.h>

class VirtualTTY: public TTYDevice, private KeyboardHandler, private Terminal::Listener {
public:
	VirtualTTY(unsigned int major, unsigned int minor);
	static DC::shared_ptr<VirtualTTY> current_tty();

	void set_active();
	bool active();
	void clear();

protected:
	static void register_tty(size_t id, VirtualTTY* device);
	static void set_current_tty(size_t tty);

private:
	static size_t _current_tty;
	static DC::shared_ptr<VirtualTTY> _ttys[NUM_TTYS];
	size_t _id;
	Terminal* terminal = nullptr;

	//KeyboardHandler
	void handle_key(KeyEvent) override;

	//TTYDevice
	size_t tty_write(const uint8_t* buffer, size_t count) override;

	//Terminal::Listener
	void on_character_change(const Terminal::Position& position, const Terminal::Character& character) override;
	void on_cursor_change(const Terminal::Position& position) override;
	void on_backspace(const Terminal::Position& position) override;
	void on_clear() override;
	void on_clear_line(size_t line) override;
	void on_scroll(size_t lines) override;
	void on_resize(const Terminal::Size& old_size, const Terminal::Size& new_size) override;

	bool _active = false;
};


#endif //DUCKOS_VIRTUALTTY_H
