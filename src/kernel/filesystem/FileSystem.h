#ifndef VFS_H
#define VFS_H

#include <kernel/device/BlockDevice.h>
#include <kernel/filesystem/Inode.h>

class Inode;
typedef uint32_t InodeID;

class Filesystem {
public:
	BlockDevice* device;
	uint8_t id;
	InodeID rootInode;
	virtual const char* name() const;
	static bool probe(BlockDevice* dev);
	virtual Inode *get_inode(InodeID id);
	Filesystem(BlockDevice* device);
};

#endif
