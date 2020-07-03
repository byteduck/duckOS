/*
    This file is part of duckOS.

    duckOS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    duckOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with duckOS.  If not, see <https://www.gnu.org/licenses/>.

    Copyright (c) Byteduck 2016-2020. All rights reserved.
*/

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
	_can_seek = !device->is_character_device();
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
	if(!_can_seek) return -ESPIPE;
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

ssize_t FileDescriptor::read(uint8_t *buffer, size_t count) {
	LOCK(lock);
	if(_seek + count < 0) return -EOVERFLOW;
	int ret = _file->read(*this, offset(), buffer, count);
	if(_can_seek && ret > 0) _seek += ret;
	return ret;
}

size_t FileDescriptor::offset() {
	return _seek;
}

ssize_t FileDescriptor::read_dir_entry(DirectoryEntry* buffer) {
	LOCK(lock);
	if(!metadata().is_directory()) return -ENOTDIR;
	ssize_t nbytes = _file->read_dir_entry(*this, offset(), buffer);
	if(nbytes > 0) {
		if(_can_seek) _seek += nbytes;
		return sizeof(DirectoryEntry) + buffer->name_length;
	}
	return nbytes;
}

ssize_t FileDescriptor::read_dir_entries(char* buffer, size_t len) {
	LOCK(lock);
	if(!metadata().is_directory()) return -ENOTDIR;
	ssize_t nbytes = 0;
	char* tmpbuf = (char*)kmalloc(sizeof(DirectoryEntry) + NAME_MAXLEN * sizeof(char));
	while(true) {
		ssize_t read = read_dir_entry((DirectoryEntry*) tmpbuf);
		if(read > 0) {
			if(read + nbytes > len) break;
			memcpy(buffer, tmpbuf, read);
			nbytes += read;
			buffer += read;
		} else if(read == 0) {
			break; //Nothing left to read
		} else {
			delete tmpbuf;
			return read; //Error
		}
	}
	delete tmpbuf;
	return nbytes;
}

ssize_t FileDescriptor::write(const uint8_t *buffer, size_t count) {
	LOCK(lock);
	if(_seek + count < 0) return -EOVERFLOW;
	int ret = _file->write(*this, offset(), buffer, count);
	if(_can_seek && ret > 0) _seek += ret;
	return ret;
}
