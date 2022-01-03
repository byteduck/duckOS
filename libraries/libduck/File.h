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
#include <memory>
#include "bits/IOBits.h"

namespace Duck {
	class File {
	public:
		static ResultRet<File> open(const Duck::Path& path, const char* mode);

		File(): File(nullptr) {}
		explicit File(FILE* file, bool close_on_destroy = false);

		ResultRet<size_t> read(void* buffer, size_t n);
		ResultRet<size_t> write(const void* buffer, size_t n);

		Result seek(long offset, Whence whence);
		Result rewind();
		off_t tell();
		[[nodiscard]] bool eof() const;
		Result flush();
		void close();

		[[nodiscard]] int fd() const;
		[[nodiscard]] FILE* c_file() const;
		[[nodiscard]] bool is_tty() const;
		[[nodiscard]] bool is_open() const;

		void set_close_on_destroy(bool close);

		[[maybe_unused]] static File std_in;
		[[maybe_unused]] static File std_out;
		[[maybe_unused]] static File std_err;

	private:
		class FileRef {
		public:
			FileRef(FILE* cfile, bool close_on_desroy):
				cfile(cfile), close_on_destroy(close_on_desroy), fd(cfile ? fileno(cfile) : -1) {}

			~FileRef() {
				if(close_on_destroy)
					close();
			}

			void close() {
				if(cfile) {
					fclose(cfile);
					cfile = nullptr;
					fd = -1;
				}
			}

			[[nodiscard]] bool is_open() const {
				return cfile;
			}

			FILE* cfile = nullptr;
			bool close_on_destroy = true;
			int fd;
		};

		std::shared_ptr<FileRef> m_fileref;
	};
}