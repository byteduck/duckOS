#ifndef DUCKOS_VFS_H
#define DUCKOS_VFS_H

#include "InodeRef.h"

class VFS {
public:
	class Mount {
	public:
		Mount();
		Mount(Filesystem* fs, InodeRef* host_inode);
		InodeID host_inode();
		Filesystem* fs();

	private:
		Filesystem* _fs;
		InodeRef* _host_inode;
	};

	static VFS& inst();

	DC::shared_ptr<InodeRef> resolve_path(string path, DC::shared_ptr<InodeRef> base);
	InodeRef& root_ref();

	VFS();
	~VFS();

	bool mount_root(Filesystem* fs);

private:
	DC::shared_ptr<Inode> _root_inode;
	DC::shared_ptr<InodeRef> _root_ref;
	Mount mounts[16];
	static VFS* instance;
};


#endif //DUCKOS_VFS_H
