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

#ifndef DUCKOS_PTYCONTROLLERDEVICE_H
#define DUCKOS_PTYCONTROLLERDEVICE_H

#include <kernel/device/CharacterDevice.h>
#include <kernel/kstd/circular_queue.hpp>

class PTYDevice;
class PTYControllerDevice: public CharacterDevice {
public:
	explicit PTYControllerDevice(unsigned int id);

	//File
	ssize_t write(FileDescriptor& fd, size_t offset, const uint8_t* buffer, size_t count) override;
	ssize_t read(FileDescriptor& fd, size_t offset, uint8_t* buffer, size_t count) override;
	bool is_pty_controller() override;
	bool can_read(const FileDescriptor& fd) override;
	bool can_write(const FileDescriptor& fd) override;

	size_t putchars(const uint8_t* buffer, size_t count);
	void notify_pty_closed();
	void ref_inc();
	void ref_dec();
	kstd::shared_ptr<PTYDevice> pty();

private:
	SpinLock _output_lock, _write_lock, _ref_lock;
	kstd::circular_queue<uint8_t> _output_buffer;
	kstd::shared_ptr<PTYDevice> _pty;
	unsigned int num_refs = 1;
};


#endif //DUCKOS_PTYCONTROLLERDEVICE_H
