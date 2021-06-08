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

#include <kernel/kstd/circular_queue.hpp>
#include <kernel/device/CharacterDevice.h>
#include <kernel/device/KeyboardDevice.h>
#include <kernel/kstd/unix_types.h>

#define NUM_TTYS 8

class TTYDevice: public CharacterDevice {
public:
	TTYDevice(unsigned major, unsigned minor);

	//Device
	ssize_t write(FileDescriptor& fd, size_t offset, const uint8_t* buffer, size_t count) override;
	ssize_t read(FileDescriptor& fd, size_t offset, uint8_t* buffer, size_t count) override;
	bool can_read(const FileDescriptor& fd) override;
	bool can_write(const FileDescriptor& fd) override;
	virtual int ioctl(unsigned request, void* argp) override;

	bool is_tty() override;
	void emit(uint8_t c);
	virtual void echo(uint8_t c) = 0;
	virtual size_t tty_write(const uint8_t* buffer, size_t count) = 0;

private:
	kstd::circular_queue<uint8_t> _input_buffer;
	BooleanBlocker _buffer_blocker;
	SpinLock _input_lock;
	pid_t _pgid = -1;
	termios _termios;
	winsize _winsize;
	int _lines = 0;

	void generate_signal(int sig);
	bool backspace();
	bool erase();
};

#endif //DUCKOS_TTYDEVICE_H
