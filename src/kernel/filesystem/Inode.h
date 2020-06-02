#ifndef DUCKOS_INODE_H
#define DUCKOS_INODE_H

#include <common.h>
#include <filesystem/FileSystem.h>

class Filesystem;
typedef uint32_t InodeID;

class Inode {
public:
	InodeID id;
	Filesystem* fs;
	bool isDirectory;
	bool isLink;

	Inode(Filesystem* fs, InodeID id);

	bool read(uint32_t start, uint32_t length, uint8_t *buffer);
};


#endif //DUCKOS_INODE_H
