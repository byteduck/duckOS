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

#include "PATADevice.h"
#include <kernel/memory/PageDirectory.h>
#include <kernel/memory/kliballoc.h>
#include <kernel/tasking/TaskManager.h>
#include <kernel/IO.h>
#include <kernel/kstd/cstring.h>
#include <kernel/tasking/Thread.h>
#include <kernel/kstd/KLog.h>

PATADevice *PATADevice::find(PATADevice::Channel channel, PATADevice::DriveType drive, bool use_pio) {
	PCI::Address addr = {0,0,0};
	PCI::enumerate_devices([](PCI::Address addr, PCI::ID id, uint16_t type, void* data) {
		if(type == PCI_TYPE_IDE_CONTROLLER)
			*((PCI::Address*)data) = addr;
	}, &addr);
	if(addr.is_zero())
		return nullptr;
	return new PATADevice(addr, channel, drive, use_pio);
}

PATADevice::PATADevice(PCI::Address addr, PATADevice::Channel channel, PATADevice::DriveType drive, bool use_pio)
	: IRQHandler()
	, DiskDevice(channel == PRIMARY ? 3 : 4, drive == MASTER ? 0 : 1)
	, _pci_addr(addr), _channel(channel), _drive(drive), _use_pio(use_pio)
{
	//IO Ports
	_io_base = channel == PRIMARY ? 0x1F0 : 0x170;
	_control_base = channel == PRIMARY ? 0x3F6 : 0x376;
	_bus_master_base = PCI::read_word(addr, PCI_BAR4) & (~1);

	//Detect bus mastering capability
	if(PCI::read_byte(addr, PCI_PROG_IF) & 0x80u) {
		KLog::dbg("PATA", "IDE controller capable of bus mastering");
	} else {
		KLog::dbg("PATA", "IDE controller not capable of bus mastering, using PIO");
		_use_pio = true;
		use_pio = true;
	}

	//IRQ
	//TODO: This will not work for multiple channels since they both use the same IRQs
	set_irq(drive == MASTER ? 14 : 15);

	//Prepare the drive
	IO::outb(_io_base + ATA_DRIVESEL, 0xA0u | (drive == SLAVE ? 0x10u : 0x00u));
	IO::outb(_io_base + ATA_SECCNT0, 0);
	IO::outb(_io_base + ATA_LBA0, 0);
	IO::outb(_io_base + ATA_LBA1, 0);
	IO::outb(_io_base + ATA_LBA2, 0);

	//Get identity
	IO::outb(_io_base + ATA_COMMAND, ATA_IDENTIFY);
	uint8_t status = wait_status();
	if(!status)
		return; //No drive found

	auto* identity = new uint16_t[256];
	auto* identity_reversed = new uint16_t[256];
	for(auto i = 0; i < 256; i++) {
		uint16_t val = IO::inw(_io_base);
		identity[i] = val;
		identity_reversed[i] = ((val & 0xFFu) << 8u) + ((val & 0xFF00u) >> 8u);
	}

	//Model is padded with spaces and not null-terminated, so let's fix that
	auto* identity_model_number = (uint8_t*) &(identity_reversed)[ATA_IDENTITY_MODEL_NUMBER_START];
	for(auto i = ATA_IDENTITY_MODEL_NUMBER_LENGTH - 1; i >= 0; i--) {
		if(identity_model_number[i] != ' ')
			break;
		identity_model_number[i] = '\0';
	}
	memcpy(_model_number, identity_model_number, ATA_IDENTITY_MODEL_NUMBER_LENGTH);

	//Detect DMA support and disk size
	auto* identity_block = (Identity*) identity;
	if(!identity_block->capabilities.dma_supported) {
		_use_pio = true;
		use_pio = true;
	}
	_max_addressable_block = identity_block->user_addressable_sectors;

	//Delete the identity buffers
	delete[] identity;
	delete[] identity_reversed;

	//Enable bus mastering and interrupt line
	PCI::enable_interrupt(addr);
	if(!use_pio) {
		PCI::enable_bus_mastering(addr);
		_prdt_region = PageDirectory::k_alloc_region(sizeof(PRDT));
		_prdt = (PRDT*) _prdt_region.virt->start;
		_dma_region = PageDirectory::k_alloc_region(ATA_MAX_SECTORS_AT_ONCE * 512);

		//Reset bus master status register
		IO::outb(_bus_master_base + ATA_BM_STATUS, IO::inb(_bus_master_base + ATA_BM_STATUS) | 0x4u);
	}

	KLog::info("PATA", "Setup disk %s using %s (%d blocks)", _model_number, _use_pio ? "PIO" : "DMA", _max_addressable_block);
}

PATADevice::~PATADevice() {
	if(_prdt_region.phys)
		PageDirectory::k_free_region(_prdt_region);
	if(_dma_region.phys)
		PageDirectory::k_free_region(_dma_region);
}

uint8_t PATADevice::wait_status(uint8_t flags) {
	uint8_t ret = 0;
	while((ret = IO::inb(_control_base)) & flags);
	return ret;
}

void PATADevice::wait_ready() {
	uint8_t status = IO::inb(_control_base);
	while((status & ATA_STATUS_BSY) || !(status & ATA_STATUS_RDY))
		status = IO::inb(_control_base);
}

Result PATADevice::read_sectors_dma(size_t lba, uint8_t num_sectors, uint8_t *buf) {
	ASSERT(num_sectors <= ATA_MAX_SECTORS_AT_ONCE);
	LOCK(_lock);

	if(num_sectors * 512 > _dma_region.phys->size)
		return false;
	_prdt->addr = _dma_region.phys->start;
	_prdt->size = num_sectors * 512;
	_prdt->eot = 0x8000;

	//Select drive and wait 10us
	IO::outb(_io_base + ATA_DRIVESEL, 0xA0u | (_drive == SLAVE ? 0x8u : 0x0u));
	IO::wait(10);

	//Stop bus master, write PRDT, clear flags, and set direction to read
	IO::outb(_bus_master_base, 0);
	IO::outl(_bus_master_base + ATA_BM_PRDT, _prdt_region.phys->start);
	IO::outb(_bus_master_base, ATA_BM_READ);
	IO::outb(_bus_master_base + ATA_BM_STATUS, IO::inb(_bus_master_base + ATA_BM_STATUS) | 0x6u);

	//Access the drive
	access_drive(ATA_READ_DMA, lba, num_sectors);

	//Wait for DRQ bit and start bus master
	while(!(IO::inb(_control_base) & ATA_STATUS_DRQ));
	IO::outb(_bus_master_base, 0x9);

	//Wait for irq
	TaskManager::current_thread()->block(_blocker);
	_blocker.set_ready(false);
	uninstall_irq();

	if(_post_irq_status & ATA_STATUS_ERR) {
		KLog::err("PATA", "DMA read fail with status 0x%x and busmaster status 0x%x", _post_irq_status, _post_irq_bm_status);
		return -EIO;
	}

	//Copy to buffer
	memcpy((void *) buf, (void*) _dma_region.virt->start, 512 * num_sectors);

	//Tell bus master we're done
	IO::outb(_bus_master_base + ATA_BM_STATUS, IO::inb(_bus_master_base + ATA_BM_STATUS) | 0x6u);

	return SUCCESS;
}

Result PATADevice::write_sectors_dma(size_t lba, uint8_t num_sectors, const uint8_t *buf) {
	ASSERT(num_sectors <= ATA_MAX_SECTORS_AT_ONCE);
	LOCK(_lock);

	if(num_sectors * 512 > _dma_region.phys->size)
		return false;
	_prdt->addr = _dma_region.phys->start;
	_prdt->size = num_sectors * 512;
	_prdt->eot = 0x8000;

	//Copy to buffer
	memcpy((void*) _dma_region.virt->start, buf, 512 * num_sectors);

	//Select drive and wait 10us
	IO::outb(_io_base + ATA_DRIVESEL, 0xA0u | (_drive == SLAVE ? 0x8u : 0x0u));
	IO::wait(10);

	//Stop bus master, write PRDT, and clear flags
	IO::outb(_bus_master_base, 0);
	IO::outl(_bus_master_base + ATA_BM_PRDT, _prdt_region.phys->start);
	IO::outb(_bus_master_base + ATA_BM_STATUS, IO::inb(_bus_master_base + ATA_BM_STATUS) | 0x6u);

	access_drive(ATA_WRITE_DMA, lba, num_sectors);

	//Wait for DRQ / not busy and start bus master
	while(IO::inb(_control_base) & ATA_STATUS_BSY || !(IO::inb(_control_base) & ATA_STATUS_DRQ));
	IO::outb(_bus_master_base, 0x1);

	//Wait for irq
	TaskManager::current_thread()->block(_blocker);
	_blocker.set_ready(false);
	uninstall_irq();

	if(_post_irq_status & ATA_STATUS_ERR) {
		KLog::err("PATA", "DMA write fail with status 0x%x and busmaster status 0x%x", _post_irq_status, _post_irq_bm_status);
		return -EIO;
	}

	//Tell bus master we're done
	IO::outb(_bus_master_base + ATA_BM_STATUS, IO::inb(_bus_master_base + ATA_BM_STATUS) | 0x6u);

	return SUCCESS;
}

void PATADevice::write_sectors_pio(uint32_t sector, uint8_t sectors, const uint8_t *buffer) {
	LOCK(_lock);

	access_drive(ATA_WRITE_PIO, sector, sectors);

	for(auto j = 0; j < sectors; j++) {
		IO::wait(10);
		while(IO::inb(_io_base + ATA_STATUS) & ATA_STATUS_BSY || !(IO::inb(_io_base + ATA_STATUS) & ATA_STATUS_DRQ));
		for(auto i = 0; i < 256; i++) {
			IO::outw(_io_base + ATA_DATA, buffer[i * 2] + (buffer[i * 2 + 1] << 8u));
		}
		TaskManager::current_thread()->block(_blocker);
		_blocker.set_ready(false);
		asm volatile("cli");
		buffer += 512;
	}
	asm volatile("sti");
	uninstall_irq();
}

void PATADevice::read_sectors_pio(uint32_t sector, uint8_t sectors, uint8_t *buffer) {
	LOCK(_lock);

	access_drive(ATA_READ_PIO, sector, sectors);

	for(auto j = 0; j < sectors; j++) {
		TaskManager::current_thread()->block(_blocker);
		_blocker.set_ready(false);
		asm volatile("cli");

		for (auto i = 0; i < 256; i++) {
			uint16_t tmp = IO::inw(_io_base + ATA_DATA);
			buffer[i * 2] = (uint8_t) tmp;
			buffer[i * 2 + 1] = (uint8_t) (tmp >> 8u);
		}
		buffer += 512;
	}

	uninstall_irq();
	asm volatile("sti");
}

void PATADevice::access_drive(uint8_t command, uint32_t lba, uint8_t num_sectors) {
	//TODO: Support 48-bit LBA
	wait_ready();

	//Select drive
	IO::outb(_io_base + ATA_DRIVESEL, 0xe0u | (_drive == SLAVE ? 0x8u : 0x0u) | ((lba & 0xF000000) >> 24));
	IO::wait(20);

	//Set count and lba
	IO::outb(_io_base + ATA_SECCNT0, num_sectors);
	IO::outb(_io_base + ATA_LBA0, (lba & 0xFFu));
	IO::outb(_io_base + ATA_LBA1, (lba & 0xFF00u) >> 8u);
	IO::outb(_io_base + ATA_LBA2, (lba & 0xFF0000u) >> 16u);

	wait_ready();

	//Install IRQ
	asm volatile("cli");
	reinstall_irq();

	//Send command
	IO::outb(_io_base + ATA_COMMAND, command);
}

Result PATADevice::read_uncached_blocks(uint32_t block, uint32_t count, uint8_t *buffer) {
	if(!_use_pio) {
		//DMA mode
		size_t num_chunks = (count + ATA_MAX_SECTORS_AT_ONCE - 1) / ATA_MAX_SECTORS_AT_ONCE;
		for (size_t i = 0; i < num_chunks; i++) {
			uint32_t num_sectors = min((size_t) ATA_MAX_SECTORS_AT_ONCE, count);
			Result res = read_sectors_dma(block + i * ATA_MAX_SECTORS_AT_ONCE, num_sectors, buffer + (512 * i * ATA_MAX_SECTORS_AT_ONCE));
			if (res.is_error()) return res;
			count -= num_sectors;
		}
		return SUCCESS;
	} else {
		//PIO mode
		while(count) {
			uint8_t to_read = min(count, 0xFFu);
			read_sectors_pio(block, to_read, buffer);
			block += to_read;
			count -= to_read;
		}
		return SUCCESS;
	}
}

Result PATADevice::write_uncached_blocks(uint32_t block, uint32_t count, const uint8_t *buffer) {
	if(!_use_pio) {
		//DMA mode
		size_t num_chunks = (count + ATA_MAX_SECTORS_AT_ONCE - 1) / ATA_MAX_SECTORS_AT_ONCE;
		for (size_t i = 0; i < num_chunks; i++) {
			uint32_t num_sectors = min((size_t) ATA_MAX_SECTORS_AT_ONCE, count);
			Result res = write_sectors_dma(block + i * ATA_MAX_SECTORS_AT_ONCE, num_sectors, buffer + (512 * i * ATA_MAX_SECTORS_AT_ONCE));
			if(res.is_error())
				return res;
			count -= num_sectors;
		}
		return SUCCESS;
	} else {
		//PIO mode
		while(count) {
			uint8_t to_read = min(count, 0xFFu);
			write_sectors_pio(block, to_read, buffer);
			block += to_read;
			count -= to_read;
		}
		return SUCCESS;
	}
}

size_t PATADevice::block_size() {
	return 512;
}

#include <kernel/filesystem/FileDescriptor.h>

ssize_t PATADevice::read(FileDescriptor &fd, size_t offset, SafePointer<uint8_t> buffer, size_t count) {
	size_t first_block = offset / block_size();
	size_t first_block_start = offset % block_size();
	size_t bytes_left = count;
	size_t block = first_block;
	ssize_t nread = 0;

	auto block_buf = new uint8_t[block_size()];
	while(bytes_left) {
		if(block > _max_addressable_block)
			break;

		Result res = read_block(block, block_buf);
		if(res.is_error()) {
			delete[] block_buf;
			return res.code();
		}

		if(block == first_block) {
			if(count < block_size() - first_block_start) {
				buffer.write(block_buf + first_block_start, count);
				nread += count;
				bytes_left = 0;
			} else {
				buffer.write(block_buf + first_block_start, block_size() - first_block_start);
				nread += block_size() - first_block_start;
				bytes_left -= block_size() - first_block_start;
			}
		} else {
			if(bytes_left < block_size()) {
				buffer.write(block_buf, count - bytes_left, bytes_left);
				nread += bytes_left;
				bytes_left = 0;
			} else {
				buffer.write(block_buf, count - bytes_left, block_size());
				nread += block_size();
				bytes_left -= block_size();
			}
		}
		block++;
	}

	delete[] block_buf;
	return nread;
}

ssize_t PATADevice::write(FileDescriptor& fd, size_t offset, SafePointer<uint8_t> buffer, size_t count) {
	size_t first_block = offset / block_size();
	size_t last_block = (offset + count) / block_size();
	size_t first_block_start = offset % block_size();
	size_t bytes_left = count;
	size_t block = first_block;

	if(last_block > _max_addressable_block)
		return -ENOSPC;

	auto block_buf = new uint8_t[block_size()];
	while(bytes_left) {
		//Read the block into a buffer
		Result res = read_block(block, block_buf);
		if(res.is_error()) {
			delete[] block_buf;
			return res.code();
		}

		//Copy the appropriate portion of the buffer into the appropriate portion of the block buffer
		if(block == first_block) {
			if(count < block_size() - first_block_start) {
				buffer.read(block_buf + first_block_start, count);
				bytes_left = 0;
			} else {
				buffer.read(block_buf + first_block_start, block_size() - first_block_start);
				bytes_left -= block_size() - first_block_start;
			}
		} else {
			if(bytes_left < block_size()) {
				buffer.read(block_buf, count - bytes_left, bytes_left);
				bytes_left = 0;
			} else {
				buffer.read(block_buf, count - bytes_left, block_size());
				bytes_left -= block_size();
			}
		}

		res = write_block(block, block_buf);
		if(res.is_error()) {
			delete[] block_buf;
			return res.code();
		}
		block++;
	}

	delete[] block_buf;
	return count;
}

void PATADevice::handle_irq(Registers *regs) {
	_post_irq_status = IO::inb(_io_base + ATA_STATUS);
	_post_irq_bm_status = IO::inb(_bus_master_base + ATA_BM_STATUS);
	if(!(_post_irq_bm_status & 0x4u))
		return; //Interrupt wasn't for this
	IO::outb(_bus_master_base + ATA_BM_STATUS, IO::inb(_bus_master_base + ATA_BM_STATUS) | 0x4u);
	_blocker.set_ready(true);
	TaskManager::yield_if_idle();
}
