/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "../tasking/Process.h"
#include "../filesystem/FileDescriptor.h"
#include <kernel/filesystem/VFS.h>

ssize_t Process::sys_read(int fd, UserspacePointer<uint8_t> buf, size_t count) {
	m_fd_lock.acquire();
	if(fd < 0 || fd >= (int) _file_descriptors.size() || !_file_descriptors[fd]) {
		m_fd_lock.release();
		return -EBADF;
	}
	auto desc = _file_descriptors[fd];
	m_fd_lock.release();

	return  desc->read(buf, count);
}

int Process::sys_readdir(int fd, UserspacePointer<char> buf, size_t len) {
	m_fd_lock.acquire();
	if(fd < 0 || fd >= (int) _file_descriptors.size() || !_file_descriptors[fd]) {
		m_fd_lock.release();
		return -EBADF;
	}
	auto desc = _file_descriptors[fd];
	m_fd_lock.release();

	return desc->read_dir_entries(buf, len);
}

ssize_t Process::sys_write(int fd, UserspacePointer<uint8_t> buffer, size_t count) {
	m_fd_lock.acquire();
	if(fd < 0 || fd >= (int) _file_descriptors.size() || !_file_descriptors[fd]) {
		m_fd_lock.release();
		return -EBADF;
	}
	auto desc = _file_descriptors[fd];
	m_fd_lock.release();

	ssize_t ret = desc->write(buffer, count);
	return ret;
}

int Process::sys_lseek(int fd, off_t off, int whence) {
	m_fd_lock.acquire();
	if(fd < 0 || fd >= (int) _file_descriptors.size() || !_file_descriptors[fd]) {
		m_fd_lock.release();
		return -EBADF;
	}
	auto desc = _file_descriptors[fd];
	m_fd_lock.release();

	return desc->seek(off, whence);
}

int Process::sys_open(UserspacePointer<char> filename, int options, int mode) {
	kstd::string path = filename.str();
	mode &= 04777; //We just want the permission bits
	auto fd_or_err = VFS::inst().open(path, options, mode & (~_umask), _user, _cwd);
	if(fd_or_err.is_error())
		return fd_or_err.code();
	LOCK(m_fd_lock);
	_file_descriptors.push_back(fd_or_err.value());
	fd_or_err.value()->set_owner(_self_ptr);
	fd_or_err.value()->set_path(path);
	fd_or_err.value()->set_id((int) _file_descriptors.size() - 1);
	return (int)_file_descriptors.size() - 1;
}

int Process::sys_close(int fd) {
	LOCK(m_fd_lock);
	if(fd < 0 || fd >= (int) _file_descriptors.size() || !_file_descriptors[fd])
		return -EBADF;
	_file_descriptors[fd] = kstd::Arc<FileDescriptor>(nullptr);
	return 0;
}