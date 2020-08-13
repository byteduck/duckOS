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

#ifndef DUCKOS_TTYDEVICE_H
#define DUCKOS_TTYDEVICE_H

#include <common/circular_queue.hpp>
#include <kernel/device/CharacterDevice.h>
#include <kernel/device/KeyboardDevice.h>
#include "Terminal.h"

#define NUM_TTYS 8

class TTYDevice: public CharacterDevice, public KeyboardHandler, private Terminal::Listener {
public:
	static DC::shared_ptr<TTYDevice> current_tty();

	TTYDevice(size_t id, const DC::string& name, unsigned major, unsigned minor);
	~TTYDevice();

	ssize_t write(FileDescriptor& fd, size_t offset, const uint8_t* buffer, size_t count) override;
	ssize_t read(FileDescriptor& fd, size_t offset, uint8_t* buffer, size_t count) override;
	bool is_tty() override;

	void set_active();
	bool active();
	void clear();
	bool buffered = true;

protected:
	static void register_tty(size_t id, TTYDevice* device);
	static void set_current_tty(size_t tty);

private:
	static size_t _current_tty;
	static DC::shared_ptr<TTYDevice> _ttys[NUM_TTYS];

	void handle_key(KeyEvent) override;

	void on_character_change(const Terminal::Position& position, const Terminal::Character& character) override;
	void on_cursor_change(const Terminal::Position& position) override;
	void on_backspace(const Terminal::Position& position) override;
	void on_clear() override;
	void on_clear_line(size_t line) override;
	void on_scroll(size_t lines) override;
	void on_resize(const Terminal::Size& old_size, const Terminal::Size& new_size);

	DC::string _name;
	DC::circular_queue<uint8_t> _input_buffer;
	DC::circular_queue<uint8_t> _buffered_input_buffer;
	Terminal* terminal;
	size_t _id;
	bool _active = false;
	BooleanBlocker _buffer_blocker;
	SpinLock _lock;
};

#endif //DUCKOS_TTYDEVICE_H