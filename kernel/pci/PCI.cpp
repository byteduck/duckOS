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

#include "PCI.h"
#include <kernel/IO.h>

namespace PCI {
	uint8_t read_byte(Address address, uint8_t field) {
		IO::outl(PCI_ADDRESS_PORT, address.get_io_address(field).value);
		return IO::inb(PCI_DATA_PORT + (field & 3));
	}

	uint16_t read_word(Address address, uint8_t field){
		IO::outl(PCI_ADDRESS_PORT, address.get_io_address(field).value);
		return IO::inw(PCI_DATA_PORT + (field & 2));
	}

	uint32_t read_dword(Address address, uint8_t field) {
		IO::outl(PCI_ADDRESS_PORT, address.get_io_address(field).value);
		return IO::inl(PCI_DATA_PORT);
	}

	void write_byte(Address address, uint8_t field, uint8_t value) {
		IO::outl(PCI_ADDRESS_PORT, address.get_io_address(field).value);
		IO::outb(PCI_DATA_PORT + (field & 3), value);
	}

	void write_word(Address address, uint8_t field, uint16_t value) {
		IO::outl(PCI_ADDRESS_PORT, address.get_io_address(field).value);
		IO::outw(PCI_DATA_PORT + (field & 2), value);
	}

	void write_dword(Address address, uint8_t field, uint32_t value) {
		IO::outl(PCI_ADDRESS_PORT, address.get_io_address(field).value);
		IO::outl(PCI_DATA_PORT, value);
	}

	void enable_interrupt(Address address) {
		Command comm = {.value = read_word(address, PCI_COMMAND)};
		comm.attrs.interrupt_disable = false;
		write_word(address, PCI_COMMAND, comm.value);
	}

	void disable_interrupt(Address address) {
		Command comm = {.value = read_word(address, PCI_COMMAND)};
		comm.attrs.interrupt_disable = true;
		write_word(address, PCI_COMMAND, comm.value);
	}

	void enable_bus_mastering(Address address) {
		Command comm = {.value = read_word(address, PCI_COMMAND)};
		comm.attrs.bus_master = true;
		write_word(address, PCI_COMMAND, comm.value);
	}

	void disable_bus_mastering(Address address) {
		Command comm = {.value = read_word(address, PCI_COMMAND)};
		comm.attrs.bus_master = false;
		write_word(address, PCI_COMMAND, comm.value);
	}

	void enumerate_devices(PCIEnumerationCallback callback, void* dataPtr) {
		if((read_byte({0,0,0}, PCI_HEADER_TYPE) & PCI_MULTIFUNCTION) == 0) {
			//Single controller
			enumerate_bus(0, callback, dataPtr);
		} else {
			// Multi controller
			for(uint8_t func = 0; func < 8; func++) {
				if(read_word({0,0, func}, PCI_VENDOR_ID) != PCI_NONE)
					enumerate_bus(func, callback, dataPtr);
			}
		}
	}

	void enumerate_bus(uint8_t bus, PCIEnumerationCallback callback, void* dataPtr) {
		for(uint8_t slot = 0; slot < 32; slot++) {
			enumerate_slot(bus, slot, callback, dataPtr);
		}
	}

	void enumerate_slot(uint8_t bus, uint8_t slot, PCIEnumerationCallback callback, void* dataPtr) {
		Address addr = {bus, slot, 0};
		if(read_word(addr, PCI_VENDOR_ID) == PCI_NONE) return;
		enumerate_functions(bus, slot, 0, callback, dataPtr);
		if(!read_byte(addr, PCI_HEADER_TYPE)) return;
		for(uint8_t func = 1; func < 8; func++) {
			if(read_word({bus, slot, func}, PCI_VENDOR_ID) != PCI_NONE) {
				enumerate_functions(bus, slot, func, callback, dataPtr);
			}
		}
	}

	void enumerate_functions(uint8_t bus, uint8_t slot, uint8_t function, PCIEnumerationCallback callback, void* dataPtr) {
		Address addr = {bus, slot, function};
		if(get_type(addr) == PCI_TYPE_BRIDGE) {
			uint8_t second_bus = read_byte(addr, PCI_SECONDARY_BUS);
			enumerate_bus(second_bus, callback, dataPtr);
		}
		callback(addr, {read_word(addr, PCI_VENDOR_ID), read_word(addr, PCI_DEVICE_ID)}, get_type(addr), dataPtr);
	}

	uint8_t get_class(Address address) {
		return PCI::read_byte(address, PCI_CLASS);
	}

	uint8_t get_subclass(Address address) {
		return PCI::read_byte(address, PCI_SUBCLASS);
	}

	uint16_t get_type(Address address) {
		return (read_byte(address, PCI_CLASS) << 8u) + read_byte(address, PCI_SUBCLASS);
	}

	IOAddress Address::get_io_address(uint8_t field) {
		return {field, function, slot, bus, true};
	}

	bool Address::is_zero() {
		return slot == 0 && function == 0 && bus == 0;
	}

	bool ID::operator==(ID &other) const {
		return other.device == device && other.vendor == vendor;
	}
}