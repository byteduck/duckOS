#ifndef DUCKOS_VFS_H
#define DUCKOS_VFS_H

#include "LinkedInode.h"
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

	DC::shared_ptr<LinkedInode> resolve_path(DC::string path, DC::shared_ptr<LinkedInode> base);
	LinkedInode& root_ref();

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
