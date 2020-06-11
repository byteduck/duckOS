#include "FileDescriptor.h"
#include "InodeFile.h"
#include "DirectoryEntry.h"
#include <common/defines.h>
#include <kernel/kstdio.h>
#include <kernel/device/Device.h>

FileDescriptor::FileDescriptor(DC::shared_ptr<File> file): _file(file.get()), _fileptr(file) {
	if(file->is_inode())
		_inode = DC::static_pointer_cast<InodeFile>(file)->inode();
}

FileDescriptor::FileDescriptor(Device* device): _file(device) {

}


void FileDescriptor::set_options(int options) {
	_readable = options & O_RDONLY;
	_writable = options & O_WRONLY;
}

bool FileDescriptor::readable() {
	return _readable;
}

bool FileDescriptor::writable() {
	return _writable;
}

int FileDescriptor::seek(int offset, int whence) {
	size_t new_seek = _seek;
	switch(whence) {
		case SEEK_SET:
			new_seek = offset;
			break;
		case SEEK_CUR:
			new_seek += offset;
			break;
		case SEEK_END:
			if(metadata().exists())
				new_seek = metadata().size;
			else return -EIO;
			break;
		default:
			return -EINVAL;
	}
	if(new_seek < 0) return -EINVAL;
	_seek = new_seek;
	return _seek;
}

InodeMetadata FileDescriptor::metadata() {
	if(_file->is_inode())
		return ((InodeFile*)_file)->inode()->metadata();
	return {};
}

size_t FileDescriptor::read(uint8_t *buffer, size_t count) {
	if(_seek + count < 0) return -EOVERFLOW;
	int ret = _file->read(*this, offset(), buffer, count);
	if(ret > 0) _seek += ret;
	return ret;
}

size_t FileDescriptor::offset() {
	return _seek;
}

size_t FileDescriptor::read_dir_entry(DirectoryEntry* buffer) {
	if(!metadata().is_directory()) return 0;
	int nbytes = _file->read_dir_entry(*this, offset(), buffer);
	if(nbytes > 0) _seek += nbytes;
	return nbytes;
}