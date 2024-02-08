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
#include <kernel/kstd/string.h>
#include <kernel/tasking/Mutex.h>

class PTYControllerDevice;
class PTYFS;
class PTYDevice: public TTYDevice {
public:
	PTYDevice(unsigned int id, const kstd::Arc<PTYControllerDevice>& controller);

	void ref_inc();
	void ref_dec();
	void notify_controller_closed();
	unsigned int id();
	kstd::string name();

	//File
	bool is_pty() override;

private:
	//TTYDevice
	size_t tty_write(const uint8_t* chars, size_t count) override;
	void echo(uint8_t c) override;

	kstd::Arc<PTYControllerDevice> _controller;
	kstd::string _name;
	unsigned int num_refs = 1;
	Mutex _lock {"PTYDevice"};
};


