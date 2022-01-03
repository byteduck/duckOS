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

#include "FileStream.h"

using namespace Duck;

[[maybe_unused]] FileInputStream FileStream::std_in(File::std_in);
[[maybe_unused]] FileOutputStream FileStream::std_out(File::std_out);
[[maybe_unused]] FileOutputStream FileStream::std_err(File::std_err);

FileStream::FileStream(): m_file(nullptr) {}
FileStream::FileStream(const File& file): m_file(file) {}
FileStream::FileStream(const Path& path) {
	open(path);
}

Result FileStream::open(const Path& path) {
	auto res = File::open(path, open_mode());
	if(res.is_error())
		return res.result();
	m_file = res.value();
	return Result::SUCCESS;
}

size_t FileInputStream::read(void* buffer, size_t n) {
	if(!m_file.c_file())
		return 0;

	auto res = m_file.read(buffer, n);
	if(res.is_error()) {
		m_err = res.result();
		return 0;
	} else {
		return res.value();
	}
}

size_t FileOutputStream::write(const void* buffer, size_t n) {
	if(!m_file.c_file())
		return 0;

	auto res = m_file.write(buffer, n);
	if(res.is_error()) {
		m_err = res.result();
		return 0;
	} else {
		return res.value();
	}
}
