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

#include "Stream.h"
#include "File.h"

namespace Duck {
	class FileInputStream;
	class FileOutputStream;

	class FileStream {
	public:
		FileStream();
		explicit FileStream(const File& file);
		explicit FileStream(const Path& path);
		virtual ~FileStream() = default;

		[[nodiscard]] const File& file() const { return m_file; }
		void set_file(const File& file) { m_file = file; }
		Result open(const Path& path);
		[[nodiscard]] bool is_open() const { return m_file.c_file(); }

		[[maybe_unused]] static FileInputStream std_in;
		[[maybe_unused]] static FileOutputStream std_out, std_err;

	protected:
		[[nodiscard]] virtual const char* open_mode() const = 0;

		File m_file;
	};

	class FileInputStream: public FileStream, public InputStream {
	public:
		FileInputStream(): FileStream() {};
		explicit FileInputStream(const File& file): FileStream(file) {}
		explicit FileInputStream(const Path& path): FileStream(path) {}

		//InputStream
		size_t read(void* buffer, size_t n) override;
		[[nodiscard]] bool eof() const override { return m_file.eof(); }
		Result seek(long seek, Whence whence) override { return m_file.seek(seek, whence); }

	protected:
		//FileStream
		[[nodiscard]] const char* open_mode() const override { return "r"; }
	};

	class FileOutputStream: public FileStream, public OutputStream {
	public:
		FileOutputStream(): FileStream() {}
		explicit FileOutputStream(const File& file): FileStream(file) {}
		explicit FileOutputStream(const Path& path): FileStream(path) {}

		//OutputStream
		size_t write(const void* buffer, size_t n) override;
		Result seek(long seek, Whence whence) override { return m_file.seek(seek, whence); }

	protected:
		//FileStream
		[[nodiscard]] const char* open_mode() const override { return "w"; }
	};
}
