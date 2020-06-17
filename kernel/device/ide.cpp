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

#include "ide.h"

namespace IDE {
	PATAChannel::PATAChannel(PCI::Address address, uint8_t channel) {
		this->address = address;
		this->channel = channel;
	}

	void PATAChannel::init() {
		PCI::enable_interrupt(address);
	}

	PATAChannel find_pata_channel(uint8_t channel) {
		PCI::Address address = {};
		PCI::enumerate_devices([](PCI::Address addr, PCI::ID id, void* address) {
			if(PCI::get_class(addr) == PCI_MASS_STORAGE_CONTROLLER && PCI::get_subclass(addr) == PCI_IDE_CONTROLLER) {
				*((PCI::Address* )address) = addr;
			}
		}, &address);
		return {address, channel};
	}
}


