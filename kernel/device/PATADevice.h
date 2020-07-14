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

#include <common/cstddef.h>
#include <common/stdlib.h>
#include <kernel/memory/MemoryRegion.h>
#include <kernel/memory/LinkedMemoryRegion.h>
#include <kernel/pci/pci.h>
#include <kernel/interrupt/IRQHandler.h>
#include <kernel/tasking/TaskYieldQueue.h>
#include "BlockDevice.h"

//Commands
#define ATA_READ_PIO          0x20
#define ATA_READ_PIO_EXT      0x24
#define ATA_READ_DMA          0xC8
#define ATA_READ_DMA_EXT      0x25
#define ATA_WRITE_PIO         0x30
#define ATA_WRITE_PIO_EXT     0x34
#define ATA_WRITE_DMA         0xCA
#define ATA_WRITE_DMA_EXT     0x35
#define ATA_CACHE_FLUSH       0xE7
#define ATA_CACHE_FLUSH_EXT   0xEA
#define ATA_PACKET            0xA0
#define ATA_IDENTIFY_PACKET   0xA1
#define ATA_IDENTIFY          0xEC

//Status
#define ATA_STATUS_ERR  0b00000001u
#define ATA_STATUS_IDX  0b00000010u
#define ATA_STATUS_CORR 0b00000100u
#define ATA_STATUS_DRQ  0b00001000u
#define ATA_STATUS_SRV  0b00010000u
#define ATA_STATUS_DF   0b00100000u
#define ATA_STATUS_RDY  0b01000000u
#define ATA_STATUS_BSY  0b10000000u

//Control registers
#define ATA_DATA        0x00
#define ATA_ERROR       0x01
#define ATA_FEATURES    0x01
#define ATA_SECCNT0     0x02
#define ATA_LBA0        0x03
#define ATA_LBA1        0x04
#define ATA_LBA2        0x05
#define ATA_DRIVESEL    0x06
#define ATA_COMMAND     0x07
#define ATA_STATUS      0x07
#define ATA_SECCNT1     0x08
#define ATA_LBA3        0x09
#define ATA_LBA4        0x0A
#define ATA_LBA5        0x0B
#define ATA_CONTROL     0x0C
#define ATA_ALTSTATUS   0x0C
#define ATA_DEVADDRESS  0x0D

//Busmaster registers
#define ATA_BM_STATUS 0x2
#define ATA_BM_PRDT 0x4

//Busmaster register values and stuff
#define ATA_BM_READ 0x8

//Other
#define ATA_IDENTITY_CYLINDERS 1
#define ATA_IDENTITY_HEADS 3
#define ATA_IDENTITY_SECTORS_PER_TRACK 6
#define ATA_IDENTITY_MODEL_NUMBER_START 54
#define ATA_IDENTITY_MODEL_NUMBER_LENGTH 40

#define ATA_MAX_SECTORS_AT_ONCE (PAGE_SIZE / 512)

class PATADevice: public IRQHandler, public BlockDevice {
public:
	typedef struct __attribute__((packed)) PRDT {
	public:
		uint32_t addr; //Address of memory region
		uint16_t size; //Size (in bytes) of region (0 means 64k)
		uint16_t eot;
	} PRDT;

	typedef enum Channel { PRIMARY, SECONDARY } Channel;
	typedef enum DriveType { MASTER, SLAVE } DriveType;

	static PATADevice* find(Channel channel, DriveType drive, bool use_pio = false);

	//PATADevice
	void io_delay();
	uint8_t wait_status(uint8_t flags = ATA_STATUS_BSY);
	void wait_ready();
	bool read_sectors_dma(size_t lba, uint16_t num_sectors, const uint8_t* buf);
	void write_sectors_pio(uint32_t sector, uint8_t sectors, const uint8_t *buffer);
	void read_sectors_pio(uint32_t sector, uint8_t sectors, uint8_t *buffer);


	//BlockDevice
	bool read_blocks(uint32_t block, uint32_t count, uint8_t *buffer) override;
	bool write_blocks(uint32_t block, uint32_t count, const uint8_t *buffer) override;
	size_t block_size() override;

	//File
	ssize_t read(FileDescriptor &fd, size_t offset, uint8_t *buffer, size_t count) override;

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
	uint8_t _use_pio = false;

	//DMA stuff
	PRDT* _prdt = nullptr;
	size_t _prdt_physaddr;
	LinkedMemoryRegion _dma_region;

	//Interrupt stuff
	TaskYieldQueue _yielder;
	uint8_t _post_irq_status;
};


#endif //DUCKOS_PATADEVICE_H
