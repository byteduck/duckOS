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

#ifndef DUCKOS_PIODEVICE_H
#define DUCKOS_PIODEVICE_H

#include <kernel/kstddef.h>
#include "BlockDevice.h"

#define PIO_DATA 0x1F0
#define PIO_ERROR 0x1F1
#define PIO_FEATURES 0x1F1
#define PIO_SECTOR_COUNT 0x1F2
#define PIO_SECTOR_LOW 0x1F3
#define PIO_SECTOR_MID  0x1F4
#define PIO_SECTOR_HIGH 0x1F5
#define PIO_DRIVE 0x1F6
#define PIO_STATUS 0x1F7
#define PIO_COMMAND 0x1F7

#define PIO_ALTERNATE_STATUS 0x3F6
#define PIO_DEVICE_CONTROL 0x3F6
#define PIO_DRIVE_ADDRESS 0x3F7

#define PIO_ERR_AMNF  0b00000001u //Address Mark Not Found
#define PIO_ERR_TKZNF 0b00000010u //Track Zero Not Found
#define PIO_ERR_ABRT  0b00000100u //Aborted Command
#define PIO_ERR_MCR   0b00001000u //Media Change Request
#define PIO_ERR_IDNF  0b00010000u //ID Not Found
#define PIO_ERR_MC    0b00100000u //Media Changed
#define PIO_ERR_UNC   0b01000000u //Uncorrectable Data Error
#define PIO_ERR_BBK   0b10000000u //Bad Block

#define PIO_STATUS_ERR  0b00000001u
#define PIO_STATUS_IDX  0b00000010u
#define PIO_STATUS_CORR 0b00000100u
#define PIO_STATUS_DRQ  0b00001000u
#define PIO_STATUS_SRV  0b00010000u
#define PIO_STATUS_DF   0b00100000u
#define PIO_STATUS_RDY  0b01000000u
#define PIO_STATUS_BSY 0b10000000u

#define PIO_READ_SECTORS 0x20
#define PIO_WRITE_SECTORS 0x30
#define PIO_CACHE_FLUSH 0xE7

class PIODevice: public BlockDevice {
public:
	PIODevice(unsigned major, unsigned minor, uint8_t id);
	uint8_t disk_id;
	bool read_blocks(uint32_t block, uint32_t count, uint8_t *buffer) override;
	bool write_blocks(uint32_t block, uint32_t count, const uint8_t *buffer) override;
	size_t block_size();
	ssize_t read(FileDescriptor& fd, size_t offset, uint8_t* buffer, size_t count) override;
	ssize_t write(FileDescriptor& fd, size_t offset, const uint8_t* buffer, size_t count) override;
	uint16_t get_first_partition();

private:
	void pio_write_sectors(uint8_t disk, uint32_t sector, uint8_t sectors, const uint8_t *buffer);
	void pio_read_sectors(uint8_t disk, uint32_t sector, uint8_t sectors, uint8_t *bufferr);
	uint16_t pio_get_first_partition(uint8_t disk);
};


#endif //DUCKOS_PIODEVICE_H
