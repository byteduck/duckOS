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

#define NUM_TTYS 8

class TTYDevice: public CharacterDevice {
public:
	TTYDevice(unsigned major, unsigned minor);

	//Device
	ssize_t write(FileDescriptor& fd, size_t offset, const uint8_t* buffer, size_t count) override;
	ssize_t read(FileDescriptor& fd, size_t offset, uint8_t* buffer, size_t count) override;
	bool can_read(const FileDescriptor& fd) override;
	bool can_write(const FileDescriptor& fd) override;

	bool is_tty() override;
	void putchar(uint8_t c);
	virtual size_t tty_write(const uint8_t* buffer, size_t count) = 0;
private:

	DC::string _name;
	DC::circular_queue<uint8_t> _input_buffer;
	DC::circular_queue<uint8_t> _buffered_input_buffer;
	BooleanBlocker _buffer_blocker;
	SpinLock _input_lock;
	bool buffered = true;
};

#endif //DUCKOS_TTYDEVICE_H
