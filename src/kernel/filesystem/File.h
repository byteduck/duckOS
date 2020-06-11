#ifndef DUCKOS_FILE_H
#define DUCKOS_FILE_H

#include <common/shared_ptr.hpp>
#include <kernel/Result.hpp>
#include "FileDescriptor.h"
#include "DirectoryEntry.h"

class FileDescriptor;
class File {
public:
	virtual ~File();
	static ResultRet<DC::shared_ptr<FileDescriptor>> open(DC::shared_ptr<File> file, int options);
	virtual bool is_inode();
	virtual size_t read(FileDescriptor& fd, size_t offset, uint8_t* buffer, size_t count);
	virtual size_t write(FileDescriptor& fd, size_t offset, const uint8_t* buffer, size_t count);
	virtual size_t read_dir_entry(FileDescriptor& fd, size_t offset, DirectoryEntry* buffer);
protected:
	File();
};


#endif //DUCKOS_FILE_H
