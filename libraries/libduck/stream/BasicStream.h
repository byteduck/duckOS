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

#include "../Result.h"
#include "../bits/IOBits.h"

namespace Duck {
	class FileInputStream;
	class FileOutputStream;

	class Stream {
	public:
		virtual ~Stream() = default;

		[[nodiscard]] Result status();

		[[maybe_unused]] static FileInputStream std_in;
		[[maybe_unused]] static FileOutputStream std_out;
		[[maybe_unused]] static FileOutputStream std_err;

	protected:
		Result m_err = 0;
	};

	class InputStream: public Stream {
	public:
		virtual size_t read(void* buffer, size_t n) = 0;

		[[nodiscard]] virtual bool eof() const = 0;
		virtual Result seek(long offset, Whence whence) = 0;

		void set_delimeter(char delimeter) { m_delimeter = delimeter; }
		[[nodiscard]] char delimeter() const { return m_delimeter; }

	private:
		char m_delimeter = '\n';
	};

	class OutputStream: public Stream {
	public:
		virtual size_t write(const void* buffer, size_t n) = 0;
		virtual Result seek(long offset, Whence whence) = 0;
	};
}