#ifndef DUCKOS_BLOCKDEVICE_H
#define DUCKOS_BLOCKDEVICE_H

#include <kernel/kstddef.h>
#include "Device.h"

class BlockDevice: public Device {
public:
	BlockDevice();
	bool read_block(uint32_t block, uint8_t *buffer);
	bool write_block(uint32_t block, uint8_t *buffer);

	virtual bool read_blocks(uint32_t block, uint32_t count, uint8_t *buffer);
	virtual bool write_blocks(uint32_t block, uint32_t count, uint8_t *buffer);

	virtual size_t block_size();
};


#endif //DUCKOS_BLOCKDEVICE_H
