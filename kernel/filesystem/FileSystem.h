#ifndef VFS_H
#define VFS_H

#include <kernel/device/BlockDevice.h>
#include <kernel/filesystem/Inode.h>
#include <common/shared_ptr.hpp>
#include <kernel/filesystem/FileDescriptor.h>

class Inode;
typedef uint32_t InodeID;

class FileDescriptor;
class Filesystem {
public:
	Filesystem(const DC::shared_ptr<FileDescriptor>& file);

	virtual char* name();
	static bool probe(DC::shared_ptr<FileDescriptor> dev);
	virtual DC::shared_ptr<Inode> get_inode(InodeID id);
	virtual Inode* get_inode_rawptr(InodeID id);
	virtual InodeID root_inode();
	virtual uint8_t fsid();
	virtual size_t block_size();
	virtual void set_block_size(size_t block_size);
	DC::shared_ptr<FileDescriptor> file_descriptor();
protected:
	DC::shared_ptr<FileDescriptor> _file;
	uint8_t _fsid;
	InodeID root_inode_id;
private:
	size_t _block_size;
};

#endif
