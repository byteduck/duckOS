#ifndef DUCKOS_PIODEVICE_H
#define DUCKOS_PIODEVICE_H

#include <kernel/kstddef.h>
#include "BlockDevice.h"

class PIODevice: public BlockDevice {
public:
	PIODevice(unsigned major, unsigned minor, uint8_t id);
	uint8_t disk_id;
	bool read_blocks(uint32_t block, uint32_t count, uint8_t *buffer);
	bool write_blocks(uint32_t block, uint32_t count, uint8_t *buffer);
	size_t block_size();
	size_t read(FileDescriptor& fd, size_t offset, uint8_t* buffer, size_t count) override;
};


#endif //DUCKOS_PIODEVICE_H
