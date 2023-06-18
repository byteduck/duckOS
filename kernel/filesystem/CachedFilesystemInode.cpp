/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "CachedFilesystemInode.h"
#include "../memory/InodeVMObject.h"

CachedFilesystemInode::CachedFilesystemInode(Filesystem& fs, ino_t id) : Inode(fs, id) {}

ssize_t CachedFilesystemInode::read_cached(size_t start, size_t length, SafePointer<uint8_t> buffer, FileDescriptor* fd) {
	auto region_err = map_inode(start, length);
	if(region_err.is_error())
		return region_err.code() < 0 ? region_err.code() : -region_err.code();
	if(!region_err.value())
		return 0;
	buffer.write((uint8_t*) (region_err.value()->start() + (start % PAGE_SIZE)), length);
	return length;
}

ssize_t CachedFilesystemInode::write_cached(size_t start, size_t length, SafePointer<uint8_t> buffer, FileDescriptor* fd) {
	auto region_err = map_inode(start, length);
	if(region_err.is_error())
		return region_err.code() < 0 ? region_err.code() : -region_err.code();
	buffer.read((uint8_t*) (region_err.value()->start() + (start % PAGE_SIZE)), length);
	// TODO: Smarter way of scheduling writes
	return write(start, length, buffer, fd);
}

ResultRet<kstd::Arc<VMRegion>> CachedFilesystemInode::map_inode(size_t start, size_t& length) {
	auto meta_size = metadata().size;
	if(start >= shared_vm_object()->size() || start >= meta_size)
		return kstd::Arc<VMRegion>(nullptr);
	if(start + length >= shared_vm_object()->size())
		length = shared_vm_object()->size() - start;
	if(start + length >= meta_size)
		length = meta_size - start;

	auto object = shared_vm_object();
	auto start_page = start / PAGE_SIZE;
	auto num_pages = (length + (start - (start_page * PAGE_SIZE)) + PAGE_SIZE - 1) / PAGE_SIZE;
	return object->map_pages_in_kernel(start_page, num_pages);
}
