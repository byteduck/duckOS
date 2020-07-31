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
#include <common/defines.h>

PATADevice *PATADevice::find(PATADevice::Channel channel, PATADevice::DriveType drive, bool use_pio) {
	PCI::Address addr = {0,0,0};
	PCI::enumerate_devices([](PCI::Address addr, PCI::ID id, uint16_t type, void* data) {
		if(type == PCI_TYPE_IDE_CONTROLLER) *((PCI::Address*)data) = addr;
	}, &addr);
	if(addr.is_zero()) return nullptr;
	return new PATADevice(addr, channel, drive, use_pio);
}

PATADevice::PATADevice(PCI::Address addr, PATADevice::Channel channel, PATADevice::DriveType drive, bool use_pio)
	: IRQHandler()
	, BlockDevice(channel == PRIMARY ? 3 : 4, drive == MASTER ? 0 : 1)
	, _pci_addr(addr), _channel(channel), _drive(drive), _use_pio(use_pio)
{
	//IO Ports
	_io_base = channel == PRIMARY ? 0x1F0 : 0x170;
	_control_base = channel == PRIMARY ? 0x3F6 : 0x376;
	_bus_master_base = PCI::read_word(addr, PCI_BAR4) & 0xFFFFFFFCU;

	//IRQ
	//TODO: This will not work for multiple channels since they both use the same IRQs
	set_irq(drive == MASTER ? 14 : 15);

	//Enable bus mastering and interrupt line
	PCI::enable_interrupt(addr);
	if(!use_pio) {
		PCI::enable_bus_mastering(addr);

		//DMA stuff
		_prdt = new PRDT; //Malloc'd memory is 4byte aligned so this is ok
		_prdt_physaddr = Memory::kernel_page_directory.get_physaddr(_prdt);
		_dma_region = PageDirectory::k_alloc_region(ATA_MAX_SECTORS_AT_ONCE * 512);
		if (!_dma_region.virt) PANIC("ATA_DMA_FAIL", "Failed to set up a DMA region for a PATA device.", true);
	}

	//Prepare the drive
	outb(_io_base + ATA_DRIVESEL, 0xA0u | (drive == SLAVE ? 0x10u : 0x00u));
	outb(_io_base + ATA_SECCNT0, 0);
	outb(_io_base + ATA_LBA0, 0);
	outb(_io_base + ATA_LBA1, 0);
	outb(_io_base + ATA_LBA2, 0);

	//Get identity
	outb(_io_base + ATA_COMMAND, ATA_IDENTIFY);
	uint8_t status = wait_status();
	if(!status) return; //No drive found

	auto* identity = (uint16_t*) kmalloc(512);
	auto* identity_reversed = (uint16_t*) kmalloc(512);
	for(auto i = 0; i < 256; i++) {
		uint16_t val = inw(_io_base);
		identity[i] = val;
		identity_reversed[i] = ((val & 0xFFu) << 8u) + ((val & 0xFF00u) >> 8u);
	}

	//Model is padded with spaces and not null-terminated, so let's fix that
	auto* identity_model_number = &((uint8_t*)identity_reversed)[ATA_IDENTITY_MODEL_NUMBER_START];
	for(auto i = ATA_IDENTITY_MODEL_NUMBER_LENGTH - 1; i >= 0; i--) {
		if(identity_model_number[i] != ' ') break;
		identity_model_number[i] = '\0';
	}
	memcpy(_model_number, identity_model_number, ATA_IDENTITY_MODEL_NUMBER_LENGTH);

	kfree(identity_reversed);
	kfree(identity);

	printf("PATA: Setup disk %s using %s\n", _model_number, _use_pio ? "PIO" : "DMA");
}

PATADevice::~PATADevice() {
	if(_prdt) delete _prdt;
}

void PATADevice::io_delay() {
	inb(_io_base + ATA_ALTSTATUS);
	inb(_io_base + ATA_ALTSTATUS);
	inb(_io_base + ATA_ALTSTATUS);
	inb(_io_base + ATA_ALTSTATUS);
}

uint8_t PATADevice::wait_status(uint8_t flags) {
	uint8_t ret = 0;
	while((ret = inb(_io_base + ATA_STATUS)) & flags);
	return ret;
}

void PATADevice::wait_ready() {
	uint8_t status = inb(_io_base + ATA_STATUS);
	while(status & ATA_STATUS_BSY || !(status & ATA_STATUS_RDY))
		status = inb(_io_base + ATA_STATUS);
}


Result PATADevice::read_sectors_dma(size_t lba, uint16_t num_sectors, uint8_t *buf) {
	ASSERT(num_sectors <= ATA_MAX_SECTORS_AT_ONCE);
	LOCK(_lock);

	if(num_sectors * 512 > _dma_region.phys->size) return false;
	_prdt->addr = _dma_region.phys->start;
	_prdt->size = num_sectors * 512;
	_prdt->eot = 0x8000;

	io_delay();

	//Stop bus master
	outb(_bus_master_base, 0);

	//Write PRDT
	outl(_bus_master_base + ATA_BM_PRDT, _prdt_physaddr);

	//Set interrupt and error flags
	outb(_bus_master_base + ATA_BM_STATUS, inb(_bus_master_base + ATA_BM_STATUS) | 0x6u);

	//Set direction to read
	outb(_bus_master_base, ATA_BM_READ);

	wait_status();

	//Select drive
	outb(_io_base + ATA_CONTROL, 0);
	outb(_io_base + ATA_DRIVESEL, 0x40u | (_drive == SLAVE ? 0x8u : 0x0u));
	io_delay();

	//Write features + Higher seccnt (We're dealing w/ 32 bit here so no need for upper stuff)
	outw(_io_base + ATA_FEATURES, 0);
	outb(_io_base + ATA_SECCNT0, 0);
	outb(_io_base + ATA_LBA0, 0);
	outb(_io_base + ATA_LBA1, 0);
	outb(_io_base + ATA_LBA2, 0);

	//Set count and lba
	outb(_io_base + ATA_SECCNT0, num_sectors);
	outb(_io_base + ATA_LBA0, (lba & 0xFFu));
	outb(_io_base + ATA_LBA1, (lba & 0xFF00u) >> 8u);
	outb(_io_base + ATA_LBA2, (lba & 0xFF0000u) >> 16u);

	wait_ready();

	//Send read DMA command
	outb(_io_base + ATA_COMMAND, ATA_READ_DMA_EXT);
	io_delay();

	//Install IRQ and start bus master
	cli();
	reinstall_irq();
	outb(_bus_master_base, 0x9);

	//Wait for irq
	TaskManager::current_process()->yield_to(_yielder);
	uninstall_irq();

	if(_post_irq_status & ATA_STATUS_ERR) return -EIO;

	//Copy to buffer
	memcpy((void *) buf, (void*) _dma_region.virt->start, 512 * num_sectors);

	//Tell bus master we're done
	outb(_bus_master_base + ATA_BM_STATUS, inb(_bus_master_base + ATA_BM_STATUS) | 0x6u);

	return SUCCESS;
}

Result PATADevice::write_sectors_dma(size_t lba, uint16_t num_sectors, const uint8_t *buf) {
	ASSERT(num_sectors <= ATA_MAX_SECTORS_AT_ONCE);
	LOCK(_lock);

	if(num_sectors * 512 > _dma_region.phys->size) return false;
	_prdt->addr = _dma_region.phys->start;
	_prdt->size = num_sectors * 512;
	_prdt->eot = 0x8000;

	//Copy to buffer
	memcpy((void*) _dma_region.virt->start, buf, 512 * num_sectors);

	io_delay();

	//Stop bus master
	outb(_bus_master_base, 0);

	//Write PRDT
	outl(_bus_master_base + ATA_BM_PRDT, _prdt_physaddr);

	//Set interrupt and error flags
	outb(_bus_master_base + ATA_BM_STATUS, inb(_bus_master_base + ATA_BM_STATUS) | 0x6u);

	wait_status();

	//Select drive
	outb(_io_base + ATA_CONTROL, 0);
	outb(_io_base + ATA_DRIVESEL, 0xe0u | (_drive == SLAVE ? 0x8u : 0x0u));
	io_delay();

	//Write features + Higher seccnt (We're dealing w/ 32 bit here so no need for upper stuff)
	outw(_io_base + ATA_FEATURES, 0);
	outb(_io_base + ATA_SECCNT0, 0);
	outb(_io_base + ATA_LBA0, 0);
	outb(_io_base + ATA_LBA1, 0);
	outb(_io_base + ATA_LBA2, 0);

	//Set count and lba
	outb(_io_base + ATA_SECCNT0, num_sectors);
	outb(_io_base + ATA_LBA0, (lba & 0xFFu));
	outb(_io_base + ATA_LBA1, (lba & 0xFF00u) >> 8u);
	outb(_io_base + ATA_LBA2, (lba & 0xFF0000u) >> 16u);

	wait_ready();

	//Send read DMA command
	outb(_io_base + ATA_COMMAND, ATA_WRITE_DMA_EXT);
	io_delay();

	//Setup IRQ and start bus master
	cli();
	reinstall_irq();
	outb(_bus_master_base, 0x1);

	//Wait for irq
	TaskManager::current_process()->yield_to(_yielder);
	uninstall_irq();

	if(_post_irq_status & ATA_STATUS_ERR) return -EIO;

	//Tell bus master we're done
	outb(_bus_master_base + ATA_BM_STATUS, inb(_bus_master_base + ATA_BM_STATUS) | 0x6u);

	return SUCCESS;
}

void PATADevice::write_sectors_pio(uint32_t sector, uint8_t sectors, const uint8_t *buffer) {
	LOCK(_lock);

	wait_status(ATA_STATUS_BSY);

	outb(_control_base, 0);
	outb(_io_base + ATA_DRIVESEL, 0xe0u | (_drive == SLAVE ? 0x8u  : 0x0u));
	io_delay();

	outb(_io_base + ATA_FEATURES, 0x0);
	outb(_io_base + ATA_SECCNT0, 0);
	outb(_io_base + ATA_LBA0, 0);
	outb(_io_base + ATA_LBA1, 0);
	outb(_io_base + ATA_LBA2, 0);

	outb(_io_base + ATA_SECCNT0, sectors);
	outb(_io_base + ATA_LBA0, (uint8_t) sector);
	outb(_io_base + ATA_LBA1, (uint8_t)(sector >> 8u));
	outb(_io_base + ATA_LBA2, (uint8_t)(sector >> 16u));

	wait_ready();

	cli();
	reinstall_irq();
	outb(_io_base + ATA_COMMAND, ATA_WRITE_PIO);

	for(auto j = 0; j < sectors; j++) {
		io_delay();
		while(inb(_io_base + ATA_STATUS) & ATA_STATUS_BSY || !(inb(_io_base + ATA_STATUS) & ATA_STATUS_DRQ));
		for(auto i = 0; i < 256; i++) {
			outw(_io_base + ATA_DATA, buffer[i * 2] + (buffer[i * 2 + 1] << 8u));
		}
		TaskManager::current_process()->yield_to(_yielder);
		cli();
		buffer += 512;
	}
	sti();
	uninstall_irq();
}

void PATADevice::read_sectors_pio(uint32_t sector, uint8_t sectors, uint8_t *buffer) {
	LOCK(_lock);

	wait_status(ATA_STATUS_BSY);

	outb(_control_base, 0);
	outb(_io_base + ATA_DRIVESEL, 0xe0u | (_drive == SLAVE ? 0x8u  : 0x0u));
	io_delay();

	outb(_io_base + ATA_FEATURES, 0x0);
	outb(_io_base + ATA_SECCNT0, 0);
	outb(_io_base + ATA_LBA0, 0);
	outb(_io_base + ATA_LBA1, 0);
	outb(_io_base + ATA_LBA2, 0);

	outb(_io_base + ATA_SECCNT0, sectors);
	outb(_io_base + ATA_LBA0, (uint8_t) sector);
	outb(_io_base + ATA_LBA1, (uint8_t)(sector >> 8u));
	outb(_io_base + ATA_LBA2, (uint8_t)(sector >> 16u));

	wait_ready();

	cli();
	reinstall_irq();
	outb(_io_base + ATA_COMMAND, ATA_READ_PIO);

	for(auto j = 0; j < sectors; j++) {
		TaskManager::current_process()->yield_to(_yielder);
		cli();

		for (auto i = 0; i < 256; i++) {
			uint16_t tmp = inw(_io_base + ATA_DATA);
			buffer[i * 2] = (uint8_t) tmp;
			buffer[i * 2 + 1] = (uint8_t) (tmp >> 8u);
		}
		buffer += 512;
	}

	uninstall_irq();
	sti();
}

Result PATADevice::read_blocks(uint32_t block, uint32_t count, uint8_t *buffer) {
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

Result PATADevice::write_blocks(uint32_t block, uint32_t count, const uint8_t *buffer) {
	if(!_use_pio) {
		//DMA mode
		size_t num_chunks = (count + ATA_MAX_SECTORS_AT_ONCE - 1) / ATA_MAX_SECTORS_AT_ONCE;
		for (size_t i = 0; i < num_chunks; i++) {
			uint32_t num_sectors = min((size_t) ATA_MAX_SECTORS_AT_ONCE, count);
			Result res = write_sectors_dma(block + i * ATA_MAX_SECTORS_AT_ONCE, num_sectors, buffer + (512 * i * ATA_MAX_SECTORS_AT_ONCE));
			if(res.is_error()) return res;
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

ssize_t PATADevice::read(FileDescriptor &fd, size_t offset, uint8_t *buffer, size_t count) {
	size_t first_block = offset / block_size();
	size_t first_block_start = offset % block_size();
	size_t bytes_left = count;
	size_t block = first_block;

	auto block_buf = new uint8_t[block_size()];
	while(bytes_left) {
		Result res = read_block(block, block_buf);
		if(res.is_error()) {
			delete[] block_buf;
			return res.code();
		}
		if(block == first_block) {
			if(count < block_size() - first_block_start) {
				memcpy(buffer, block_buf + first_block_start, count);
				bytes_left = 0;
			} else {
				memcpy(buffer, block_buf + first_block_start, block_size() - first_block_start);
				bytes_left -= block_size() - first_block_start;
			}
		} else {
			if(bytes_left < block_size()) {
				memcpy(buffer + (count - bytes_left), block_buf, bytes_left);
				bytes_left = 0;
			} else {
				memcpy(buffer + (count - bytes_left), block_buf, block_size());
				bytes_left -= block_size();
			}
		}
		block++;
	}

	delete[] block_buf;
	return count;
}

ssize_t PATADevice::write(FileDescriptor& fd, size_t offset, const uint8_t* buffer, size_t count) {
	size_t first_block = offset / block_size();
	size_t first_block_start = offset % block_size();
	size_t bytes_left = count;
	size_t block = first_block;

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
				memcpy(block_buf + first_block_start, buffer, count);
				bytes_left = 0;
			} else {
				memcpy(block_buf + first_block_start, buffer, block_size() - first_block_start);
				bytes_left -= block_size() - first_block_start;
			}
		} else {
			if(bytes_left < block_size()) {
				memcpy(block_buf, buffer + (count - bytes_left), bytes_left);
				bytes_left = 0;
			} else {
				memcpy(block_buf, buffer + (count - bytes_left), block_size());
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
	_post_irq_status = inb(_io_base + ATA_STATUS);
	uint8_t bus_status = inb(_bus_master_base + ATA_BM_STATUS);
	if(!(bus_status & 0x4u)) return; //Interrupt wasn't for this
	_yielder.set_ready();
}
