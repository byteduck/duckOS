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

#ifndef DUCKOS_PCI_H
#define DUCKOS_PCI_H

#include <kernel/kstd/kstddef.h>

//PORTS
#define PCI_ADDRESS_PORT 0xCF8
#define PCI_DATA_PORT 0xCFC

//Fields
#define PCI_VENDOR_ID 0x0 //16
#define PCI_DEVICE_ID 0x2 //16
#define PCI_COMMAND 0x4 //16
#define PCI_STATUS 0x6 //16
#define PCI_REVISION_ID 0x8 //8
#define PCI_PROG_IF 0x9 //8
#define PCI_SUBCLASS 0xa //8
#define PCI_CLASS 0xb //8
#define PCI_CACHE_LINE_SIZE 0xc //8
#define PCI_LATENCY_TIMER 0xd //8
#define PCI_HEADER_TYPE 0xe //8
#define PCI_BIST 0xf //8
#define PCI_BAR0 0x10 //32
#define PCI_BAR1 0x14 //32
#define PCI_BAR2 0x18 //32
#define PCI_BAR3 0x1C //32
#define PCI_BAR4 0x20 //32
#define PCI_BAR5 0x24 //32
#define PCI_PRIMARY_BUS 0x18 //8
#define PCI_SECONDARY_BUS 0x19 //8
#define PCI_INTERRUPT_LINE 0x3c //8
#define PCI_INTERRUPT_PIN 0x3d //8

//Header types
#define PCI_MULTIFUNCTION 0x80

//Vendors
#define PCI_NONE 0xFFFF

//Device Classes
#define PCI_UNCLASSIFIED 0x0
#define PCI_MASS_STORAGE_CONTROLLER 0x1
#define PCI_NETWORK_CONTROLLER 0x2
#define PCI_DISPLAY_CONTROLLER 0x3
#define PCI_MULTIMEDIA_CONTROLLER 0x4
#define PCI_MEMORY_CONTROLLER 0x5
#define PCI_BRIDGE_DEVICE 0x6
#define PCI_SIMPLE_COMMUNICATION_CONTROLLER 0x7
#define PCI_BASE_SYSTEM_PERIPHERAL 0x8
#define PCI_INPUT_DEVICE_CONTROLLER 0x9
#define PCI_DOCKING_STATION 0xA
#define PCI_PROCESSOR 0xB
#define PCI_SERIAL_BUS_CONTROLLER 0xC
#define PCI_WIRELESS_CONTROLLER 0xD
#define PCI_INTELLIGENT_CONTROLLER 0xE
#define PCI_SATELLITE_COMMUNICATION_CONTROLLER 0xF
#define PCI_ENCRYPTION_CONTROLLER 0x10
#define PCI_SIGNAL_PROCESSING_CONTROLLER 0x11
#define PCI_PROCESSING_ACCELERATOR 0x12
#define PCI_NON_ESSENTIAL_INSTRUMENTATION 0x13
#define PCI_COPROCESSOR 0x40
#define PCI_UNASSIGNED_CLASS 0xFF

//Device Subclasses
#define PCI_PCI_BRIDGE 0x4
#define PCI_IDE_CONTROLLER 0x1

//Device types
#define PCI_TYPE_BRIDGE 0x0604
#define PCI_TYPE_IDE_CONTROLLER 0x0101

namespace PCI {
	union IOAddress {
		struct __attribute((packed)) attrs {
			uint8_t field: 8;
			uint8_t function: 3;
			uint8_t slot: 5;
			uint8_t bus: 8;
			uint8_t : 7;
			bool enable: 1;
		} attrs;
		uint32_t value;
	};

	union Command {
		struct __attribute((packed)) attrs {
			bool io_space : 1;
			bool mem_space : 1;
			bool bus_master : 1;
			bool special_cycles : 1;
			bool mem_write_invalidate_enable : 1;
			bool VGA_palette_snoop : 1;
			bool parity_error_response : 1;
			bool : 1;
			bool serr_enable : 1;
			bool fast_back_to_back_enable : 1;
			bool interrupt_disable : 1;
			uint8_t reserved2 : 5;
		} attrs;
		uint16_t value;
	};

	class Address {
	public:
		uint8_t bus;
		uint8_t slot;
		uint8_t function;
		IOAddress get_io_address(uint8_t field);
		bool is_zero();
	};

	class ID {
	public:
		uint16_t vendor;
		uint16_t device;
		bool operator==(ID& other) const;
	};

	typedef void (*PCIEnumerationCallback)(Address addr, ID id, uint16_t type, void* dataPtr);

	uint8_t read_byte(Address address, uint8_t field);
	uint16_t read_word(Address address, uint8_t field);
	uint32_t read_dword(Address address, uint8_t field);

	void write_byte(Address address, uint8_t field, uint8_t value);
	void write_word(Address address, uint8_t field, uint16_t value);
	void write_dword(Address address, uint8_t field, uint32_t value);

	void enable_interrupt(Address address);
	void disable_interrupt(Address address);
	void enable_bus_mastering(Address address);
	void disable_bus_mastering(Address address);

	void enumerate_devices(PCIEnumerationCallback callback, void* dataPtr);
	void enumerate_bus(uint8_t bus, PCIEnumerationCallback callback, void* dataPtr);
	void enumerate_slot(uint8_t bus, uint8_t slot, PCIEnumerationCallback callback, void* dataPtr);
	void enumerate_functions(uint8_t bus, uint8_t slot, uint8_t function, PCIEnumerationCallback callback, void* dataPtr);

	uint8_t get_class(Address address);
	uint8_t get_subclass(Address address);
	uint16_t get_type(Address address);

};


#endif //DUCKOS_PCI_H
