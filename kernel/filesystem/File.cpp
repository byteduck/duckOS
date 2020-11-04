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

#include <kernel/kstdio.h>
#include <common/defines.h>
#include "File.h"

File::File() {

}

File::~File() {

}

bool File::is_inode() {
	return false;
}

bool File::is_tty() {
	return false;
}

bool File::is_pty_controller() {
	return false;
}

bool File::is_pty_mux() {
	return false;
}

bool File::is_pty() {
	return false;
}

bool File::is_fifo() {
	return false;
}

ssize_t File::read(FileDescriptor &fd, size_t offset, uint8_t *buffer, size_t count) {
	return 0;
}

ssize_t File::write(FileDescriptor& fd, size_t offset, const uint8_t* buffer, size_t count) {
	return 0;
}


ssize_t File::read_dir_entry(FileDescriptor &fd, size_t offset, DirectoryEntry *buffer) {
	return 0;
}

int File::ioctl(unsigned request, void* argp) {
	return -ENOTTY;
}

void File::open(FileDescriptor& fd, int options) {

}

void File::close(FileDescriptor& fd) {

}

bool File::can_read(const FileDescriptor& fd) {
	return true;
}

bool File::can_write(const FileDescriptor& fd) {
	return true;
}

