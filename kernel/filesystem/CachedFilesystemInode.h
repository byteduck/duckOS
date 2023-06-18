/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include "Inode.h"

class CachedFilesystemInode: public Inode {
public:
	CachedFilesystemInode(Filesystem& fs, ino_t id);

	ssize_t read_cached(size_t start, size_t length, SafePointer<uint8_t> buffer, FileDescriptor* fd) override;
	ssize_t write_cached(size_t start, size_t length, SafePointer<uint8_t> buffer, FileDescriptor* fd) override;

private:
	ResultRet<kstd::Arc<VMRegion>> map_inode(size_t start, size_t& length);
};
