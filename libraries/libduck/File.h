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

#pragma once

#include "Result.h"
#include "Path.h"
#include <cstdio>
#include <unistd.h>
#include "bits/IOBits.h"

namespace Duck {
	class File {
	public:
		static ResultRet<File> open(const Duck::Path& path, const char* mode);

		File(): File(nullptr) {}
		explicit File(FILE* file);

		ResultRet<size_t> read(void* buffer, size_t n);
		ResultRet<size_t> write(const void* buffer, size_t n);

		Result seek(long offset, Whence whence) { return fseek(m_cfile, offset, whence) ? Result(ferror(m_cfile)) : Result::SUCCESS; }
		Result rewind() { return seek(0, SET); }
		off_t tell() { return ftell(m_cfile); }
		[[nodiscard]] bool eof() const { return feof(m_cfile); }
		Result flush() { return fflush(m_cfile) ? Result(ferror(m_cfile)) : Result::SUCCESS; }

		[[nodiscard]] int fd() const { return m_fd; }
		[[nodiscard]] FILE* c_file() const { return m_cfile; }
		[[nodiscard]] bool is_tty() const { return isatty(m_fd); }

		[[maybe_unused]] static File std_in;
		[[maybe_unused]] static File std_out;
		[[maybe_unused]] static File std_err;

	private:
		FILE* m_cfile;
		int m_fd;
	};
}