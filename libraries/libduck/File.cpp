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

	Copyright (c) Byteduck 2016-2021. All rights reserved.
*/

#include "File.h"
using namespace Duck;

[[maybe_unused]] File File::std_in(stdin);
[[maybe_unused]] File File::std_out(stdout);
[[maybe_unused]] File File::std_err(stderr);

ResultRet<File> File::open(const Path& path, const char* mode) {
	auto c_file = fopen(path.string().c_str(), mode);
	if(!c_file)
		return Result(errno);
	else
		return File(c_file, true);
}

File::File(FILE* file, bool close_on_destroy):
	m_fileref(std::make_shared<FileRef>(file, close_on_destroy)) {}

ResultRet<size_t> File::read(void* buffer, size_t n) {
	if(!m_fileref->is_open())
		return Result {EBADF};

	size_t res = fread(buffer, 1, n, m_fileref->cfile);
	if(ferror(m_fileref->cfile)) {
		clearerr(m_fileref->cfile);
		return Result(errno);
	} else {
		return res;
	}
}

ResultRet<std::string> File::read_all() {
	if(!m_fileref->is_open())
		return Result {EBADF};

	size_t size = stat().st_size;
	std::string buf;
	buf.resize(size + 1);
	auto res = fread((void*) buf.c_str(), 1, size, m_fileref->cfile);
	if(int err = ferror(m_fileref->cfile)) {
		clearerr(m_fileref->cfile);
		return Result(err);
	}
	buf.resize(res + 1, '\0');
	return buf;
}

ResultRet<size_t> File::write(const void* buffer, size_t n) {
	if(!m_fileref->is_open())
		return Result {EBADF};

	size_t res = fwrite(buffer, 1, n, m_fileref->cfile);
	if(ferror(m_fileref->cfile)) {
		clearerr(m_fileref->cfile);
		return Result(errno);
	} else {
		return res;
	}
}

Result File::seek(long offset, Whence whence) {
	if(!m_fileref->is_open())
		return {EBADF};
	return fseek(m_fileref->cfile, offset, whence) ? Result(ferror(m_fileref->cfile)) : Result::SUCCESS;

}
Result File::rewind() {
	return seek(0, SET);
}

off_t File::tell() {
	if(!m_fileref->is_open())
		return -1;
	return ftell(m_fileref->cfile);
}

[[nodiscard]] bool File::eof() const {
	if(!m_fileref->is_open())
		return false;
	return feof(m_fileref->cfile);
}

Result File::flush() {
	if(!m_fileref->is_open())
		return {EBADF};
	return fflush(m_fileref->cfile) ? Result(ferror(m_fileref->cfile)) : Result::SUCCESS;
}

void File::close() {
	m_fileref->close();
}

[[nodiscard]] int File::fd() const {
	return m_fileref->fd;
}

[[nodiscard]] FILE* File::c_file() const {
	return m_fileref->cfile;
}

[[nodiscard]] bool File::is_tty() const {
	if(!m_fileref->is_open())
		return false;
	return isatty(m_fileref->fd);
}

bool File::is_open() const {
	return m_fileref->is_open();
}

struct stat File::stat() const {
	struct stat ret;
	fstat(m_fileref->fd, &ret);
	return ret;
}

void File::set_close_on_destroy(bool close) {
	m_fileref->close_on_destroy = close;
}
