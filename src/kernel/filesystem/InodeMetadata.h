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

	bool is_directory();
	bool is_simple_file();
	bool exists();
};


#endif //DUCKOS_INODEMETADATA_H
