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

#ifndef DUCKOS_DEVICE_H
#define DUCKOS_DEVICE_H

#include <kernel/filesystem/File.h>
#include <kernel/kstd/vector.hpp>

class Device: public File {
public:
	static void init();
	static kstd::vector<kstd::shared_ptr<Device>> devices();
	static ResultRet<kstd::shared_ptr<Device>> get_device(unsigned major, unsigned minor);
	static void remove_device(unsigned major, unsigned minor);

	virtual ~Device() override;

	unsigned major();
	unsigned minor();
	kstd::shared_ptr<Device> shared_ptr();

	virtual bool is_block_device();
	virtual bool is_character_device();

protected:
	Device(unsigned major, unsigned minor);

private:
	static kstd::vector<kstd::shared_ptr<Device>> _devices;
	static SpinLock _lock;

	unsigned _major = 0;
	unsigned _minor = 0;
};

#endif //DUCKOS_DEVICE_H
