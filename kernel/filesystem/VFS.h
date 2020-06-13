#ifndef DUCKOS_VFS_H
#define DUCKOS_VFS_H

#include "LinkedInode.h"
#include "FileDescriptor.h"
#include <common/string.h>

class VFS {
public:
	class Mount {
	public:
		Mount();
		Mount(Filesystem* fs, LinkedInode* host_inode);
		InodeID host_inode();
		Filesystem* fs();

	private:
		Filesystem* _fs;
		LinkedInode* _host_inode;
	};

	static VFS& inst();

	ResultRet<DC::shared_ptr<LinkedInode>> resolve_path(DC::string path, const DC::shared_ptr<LinkedInode>& base, DC::shared_ptr<LinkedInode>* parent_storage);
	ResultRet<DC::shared_ptr<FileDescriptor>> open(DC::string path, int options, int mode, DC::shared_ptr<LinkedInode> base);
	DC::shared_ptr<LinkedInode> root_ref();

	VFS();
	~VFS();

	bool mount_root(Filesystem* fs);

private:
	DC::shared_ptr<Inode> _root_inode;
	DC::shared_ptr<LinkedInode> _root_ref;
	Mount mounts[16];
	static VFS* instance;
};


#endif //DUCKOS_VFS_H
