#ifndef DUCKOS_FILEDESCRIPTOR_H
#define DUCKOS_FILEDESCRIPTOR_H

#include <common/shared_ptr.hpp>
#include "File.h"
#include "Inode.h"
#include "InodeMetadata.h"
#include "DirectoryEntry.h"

class File;
class DirectoryEntry;
class Device;
class FileDescriptor {
public:
	FileDescriptor(DC::shared_ptr<File> file);
	FileDescriptor(Device* device);

	void set_options(int options);
	bool readable();
	bool writable();
	InodeMetadata metadata();

	int seek(int offset, int whence);
	ssize_t read(uint8_t* buffer, size_t count);
	ssize_t read_dir_entry(DirectoryEntry *buffer);
	ssize_t write(const uint8_t* buffer, size_t count);
	size_t offset();
private:
	DC::shared_ptr<File> _fileptr;
	File* _file;
	DC::shared_ptr<Inode> _inode;

	bool _readable {false};
	bool _writable {false};
	bool _can_seek {true};

	int _seek {0};
};


#endif //DUCKOS_FILEDESCRIPTOR_H
