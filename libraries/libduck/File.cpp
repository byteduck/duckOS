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
		return File(c_file);
}

File::File(FILE* file): m_cfile(file), m_fd(file ? fileno(file) : -1) {}

ResultRet<size_t> File::read(void* buffer, size_t n) {
	size_t res = fread(buffer, 1, n, m_cfile);
	if(ferror(m_cfile)) {
		clearerr(m_cfile);
		return Result(errno);
	} else {
		return res;
	}
}

ResultRet<size_t> File::write(const void* buffer, size_t n) {
	size_t res = fwrite(buffer, 1, n, m_cfile);
	if(ferror(m_cfile)) {
		clearerr(m_cfile);
		return Result(errno);
	} else {
		return res;
	}
}
