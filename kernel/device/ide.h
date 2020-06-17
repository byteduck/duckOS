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

#ifndef DUCKOS_IDE_H
#define DUCKOS_IDE_H

#include <kernel/pci/pci.h>

#define ATA_PRIMARY 0x0
#define ATA_SECONDARY 0x1

namespace IDE {
	class PATAChannel {
	public:
		PCI::Address address;
		uint8_t channel;
		PATAChannel(PCI::Address address, uint8_t channel);
		void init();
	};

	PATAChannel find_pata_channel(uint8_t channel);
}

#endif //DUCKOS_IDE_H
