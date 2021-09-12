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

#ifndef DUCKOS_PATADEVICE_H
#define DUCKOS_PATADEVICE_H

#include <kernel/kstd/types.h>
#include <kernel/kstd/kstdlib.h>
#include <kernel/pci/PCI.h>
#include <kernel/interrupt/IRQHandler.h>
#include "ATA.h"
#include "DiskDevice.h"
#include <kernel/tasking/SpinLock.h>
#include <kernel/memory/LinkedMemoryRegion.h>
#include <kernel/memory/MemoryManager.h>

#define ATA_MAX_SECTORS_AT_ONCE (PAGE_SIZE / 512)

class PATADevice: public IRQHandler, public DiskDevice {
public:
	typedef enum Channel { PRIMARY, SECONDARY } Channel;
	typedef enum DriveType { MASTER, SLAVE } DriveType;

	static PATADevice* find(Channel channel, DriveType drive, bool use_pio = false);

	//PATADevice
	~PATADevice();
	uint8_t wait_status(uint8_t flags = ATA_STATUS_BSY);
	void wait_ready();
	Result read_sectors_dma(uint32_t sector, uint8_t num_sectors, uint8_t* buf);
	Result write_sectors_dma(uint32_t sector, uint8_t num_sectors, const uint8_t* buf);
	void read_sectors_pio(uint32_t sector, uint8_t sectors, uint8_t *buffer);
	void write_sectors_pio(uint32_t sector, uint8_t sectors, const uint8_t *buffer);
	void access_drive(uint8_t command, uint32_t lba, uint8_t num_sectors);


	//BlockDevice
	Result read_uncached_blocks(uint32_t block, uint32_t count, uint8_t *buffer) override;
	Result write_uncached_blocks(uint32_t block, uint32_t count, const uint8_t *buffer) override;
	size_t block_size() override;

	//File
	ssize_t read(FileDescriptor& fd, size_t offset, uint8_t* buffer, size_t count) override;
	ssize_t write(FileDescriptor& fd, size_t offset, const uint8_t* buffer, size_t count) override;

	//IRQHandler
	void handle_irq(Registers* regs) override;

private:
	PATADevice(PCI::Address addr, Channel channel, DriveType drive, bool use_pio);

	//Addresses
	PCI::Address _pci_addr;
	uint16_t _io_base;
	uint16_t _control_base;
	uint16_t _bus_master_base;

	//Drive info
	Channel _channel;
	DriveType _drive;
	char _model_number[40];
	bool _use_pio = false;
	uint64_t _max_addressable_block;

	//DMA stuff
	PRDT* _prdt = nullptr;
	LinkedMemoryRegion _dma_region;
	LinkedMemoryRegion _prdt_region;

	//Interrupt stuff
	BooleanBlocker _blocker;
	uint8_t _post_irq_status, _post_irq_bm_status;
	volatile bool _got_irq = false;

	//Lock
	SpinLock _lock;
};


#endif //DUCKOS_PATADEVICE_H
