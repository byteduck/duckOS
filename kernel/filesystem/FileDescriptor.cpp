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
#include "Pipe.h"
#include <common/defines.h>
#include <kernel/kstdio.h>
#include <kernel/device/Device.h>

FileDescriptor::FileDescriptor(const DC::shared_ptr<File>& file, User& user): _file(file), _user(user) {
	if(file->is_inode())
		_inode = DC::static_pointer_cast<InodeFile>(file)->inode();
}

FileDescriptor::FileDescriptor(FileDescriptor& other): _user(other._user) {
	_file = other._file;
	_is_fifo_writer = other._is_fifo_writer;
	_append = other._append;
	_can_seek = other._can_seek;
	_inode = other._inode;
	_readable = other._readable;
	_writable = other._writable;
	_seek = other._seek;
	_options = other._options;

	//Increase pipe reader/writer count if applicable
	if(_file->is_fifo()) {
		if (_is_fifo_writer) {
			((Pipe*) _file.get())->add_writer();
		} else {
			((Pipe*) _file.get())->add_reader();
		}
	}

	open();
}

FileDescriptor::~FileDescriptor() {
	//Decrease pipe reader/writer count if applicable
	if(_file->is_fifo()) {
		if (_is_fifo_writer) {
			((Pipe*) _file.get())->remove_writer();
		} else {
			((Pipe*) _file.get())->remove_reader();
		}
	}

	_file->close(*this);
}


void FileDescriptor::set_options(int options) {
	_readable = !(options & O_WRONLY) || (options & O_RDWR);
	_writable = (options & O_WRONLY) || (options & O_RDWR);
	_append = options & O_APPEND;
	_options = options;
}

bool FileDescriptor::readable() {
	return _readable;
}

bool FileDescriptor::writable() {
	return _writable;
}

bool FileDescriptor::append_mode() {
	return _append;
}

int FileDescriptor::seek(off_t offset, int whence) {
	if(!_can_seek) return -ESPIPE;
	off_t new_seek = _seek;
	switch(whence) {
		case SEEK_SET:
			new_seek = offset;
			break;
		case SEEK_CUR:
			new_seek += offset;
			break;
		case SEEK_END:
			if(metadata().exists())
				new_seek = metadata().size + offset;
			else return -EIO;
			break;
		default:
			return -EINVAL;
	}
	if(new_seek < 0) return -EINVAL;
	if(metadata().exists() && metadata().is_device() && new_seek > metadata().size) return -EINVAL;
	_seek = new_seek;
	return _seek;
}

InodeMetadata FileDescriptor::metadata() {
	if(_file->is_inode())
		return ((InodeFile*)_file.get())->inode()->metadata();
	return {};
}

DC::shared_ptr<File> FileDescriptor::file() {
	return _file;
}

void FileDescriptor::open() {
	_file->open(*this, _options);
}

ssize_t FileDescriptor::read(uint8_t *buffer, size_t count) {
	if(!_readable) return -EBADF;
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
	if(!_readable) return -EBADF;
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
	if(!_readable) return -EBADF;
	LOCK(lock);
	if(!metadata().is_directory()) return -ENOTDIR;
	ssize_t nbytes = 0;
	auto* dirbuf = new DirectoryEntry;
	while(true) {
		ssize_t read = _file->read_dir_entry(*this, offset(), dirbuf);
		if(read > 0) {
			if(_can_seek) _seek += read;
			if(read + nbytes > len) break;
			size_t entry_len = dirbuf->entry_length();
			memcpy(buffer, dirbuf, entry_len);
			nbytes += entry_len;
			buffer += entry_len;
		} else if(read == 0) {
			break; //Nothing left to read
		} else {
			delete dirbuf;
			return read; //Error
		}
	}
	delete dirbuf;
	return nbytes;
}

ssize_t FileDescriptor::write(const uint8_t *buffer, size_t count) {
	if(!_writable) return -EBADF;
	LOCK(lock);
	if(_append && _can_seek && metadata().exists()) _seek = metadata().size;
	if(_seek + count < 0) return -EOVERFLOW;
	int ret = _file->write(*this, offset(), buffer, count);
	if(_can_seek && ret > 0) _seek += ret;
	return ret;
}

int FileDescriptor::ioctl(unsigned request, void* argp) {
	return _file->ioctl(request, argp);
}

void FileDescriptor::set_fifo_reader() {
	_is_fifo_writer = false;
}

void FileDescriptor::set_fifo_writer() {
	_is_fifo_writer = true;
}

bool FileDescriptor::is_fifo_writer() {
	return _is_fifo_writer;
}
