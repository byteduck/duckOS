#ifndef DUCKOS_FILEDESCRIPTOR_H
#define DUCKOS_FILEDESCRIPTOR_H

#include <common/shared_ptr.hpp>
#include "File.h"
#include "Inode.h"
#include "InodeMetadata.h"
#include "DirectoryEntry.h"

class File;
class DirectoryEntry;
class FileDescriptor {
public:
	FileDescriptor(DC::shared_ptr<File> file);

	void set_options(int options);
	bool readable();
	bool writable();
	InodeMetadata metadata();

	int seek(int offset, int whence);
	size_t read(uint8_t* buffer, size_t count);
	size_t read_dir_entry(DirectoryEntry *buffer);
	size_t offset();
private:
	DC::shared_ptr<File> _file;
	DC::shared_ptr<Inode> _inode;

	bool _readable {false};
	bool _writable {false};

	int _seek {0};
};


#endif //DUCKOS_FILEDESCRIPTOR_H
