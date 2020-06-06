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

    Inode(Filesystem* fs, InodeID id);
    Inode();

	virtual bool is_directory();
	virtual bool is_link();
	virtual Inode* find(string name);
	virtual bool read(uint32_t start, uint32_t length, uint8_t *buffer);
	bool is_null();
};


#endif //DUCKOS_INODE_H
