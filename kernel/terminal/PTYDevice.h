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

#ifndef DUCKOS_PTYDEVICE_H
#define DUCKOS_PTYDEVICE_H

#include "TTYDevice.h"

class PTYControllerDevice;
class PTYFS;
class PTYDevice: public TTYDevice {
public:
	PTYDevice(unsigned int id, PTYControllerDevice* controller);

	void ref_inc();
	void ref_dec();
	void notify_controller_closed();
	unsigned int id();
	DC::string name();

	//File
	bool is_pty();

private:
	//TTYDevice
	size_t tty_write(const uint8_t* chars, size_t count) override;
	void echo(uint8_t c) override;

	PTYControllerDevice* _controller;
	DC::string _name;
	unsigned int num_refs = 1;
	SpinLock _lock;
};


#endif //DUCKOS_PTYDEVICE_H
