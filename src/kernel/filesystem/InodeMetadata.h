#ifndef DUCKOS_INODEMETADATA_H
#define DUCKOS_INODEMETADATA_H

#include <common/cstddef.h>

#define MODE_FIFO 0x1000
#define MODE_CHAR_DEVICE 0x2000
#define MODE_DIRECTORY 0x4000
#define MODE_BLOCK_DEVICE 0x6000
#define MODE_FILE 0x8000
#define MODE_SYMLINK 0xA000
#define MODE_SOCKET 0xC000

typedef uint32_t InodeID;
class InodeMetadata {
public:
	size_t mode = 0;
	size_t size = 0;
	InodeID inode_id;

	unsigned dev_major;
	unsigned dev_minor;

	bool is_directory() const;
	bool is_block_device() const;
	bool is_character_device() const;
	bool is_device() const;
	bool is_simple_file() const;
	bool exists() const;
};


#endif //DUCKOS_INODEMETADATA_H
