#include <kstdio.h>
#include "PIODevice.h"
#include "ata.h"

PIODevice::PIODevice(uint8_t id) {
	disk_id = id;
}

bool PIODevice::read_blocks(uint32_t block, uint32_t count, uint8_t *buffer) {
	pio_read_sectors(disk_id, block, count, buffer);
	return true;
}

bool PIODevice::write_blocks(uint32_t block, uint32_t count, uint8_t *buffer) {
	//TODO: Writing
	return false;
}

size_t PIODevice::block_size() {
	return 512;
}
