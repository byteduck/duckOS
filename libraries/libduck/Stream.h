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
#include "bits/IOBits.h"

namespace Duck {
	class InputStream;
	class OutputStream;

	class Stream {
	public:
		virtual ~Stream();

		[[nodiscard]] Result status();

		[[maybe_unused]] static InputStream& std_in;
		[[maybe_unused]] static OutputStream& std_out;
		[[maybe_unused]] static OutputStream& std_err;

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
		char getchar() {
			char ret;
			read(&ret, 1);
			return ret;
		}

	private:
		char m_delimeter = '\n';
	};

	class OutputStream: public Stream {
	public:
		virtual size_t write(const void* buffer, size_t n) = 0;
		virtual Result seek(long offset, Whence whence) = 0;
		bool putchar(char chr) { return write(&chr, 1); }
	};

	/*
	 * InputStream stuff
	 */

	InputStream& operator>>(InputStream& stream, uint8_t& byte);
	InputStream& operator>>(InputStream& stream, char& chr);
	InputStream& operator>>(InputStream& stream, std::string& string);

	/*
	 * OutputStream strings
	 */

	OutputStream& operator<<(OutputStream& stream, const char* chars);
	OutputStream& operator<<(OutputStream& stream, const std::string& string);
	OutputStream& operator<<(OutputStream& stream, const std::string_view& view);

	/*
	 * OutputStream primitives
	 */

	OutputStream& operator<<(OutputStream& stream, const uint8_t& byte);
	OutputStream& operator<<(OutputStream& stream, int value);
	OutputStream& operator<<(OutputStream& stream, long value);
	OutputStream& operator<<(OutputStream& stream, float value);
	OutputStream& operator<<(OutputStream& stream, double value);
	OutputStream& operator<<(OutputStream& stream, long long value);
	OutputStream& operator<<(OutputStream& stream, long double value);
	OutputStream& operator<<(OutputStream& stream, unsigned int value);
	OutputStream& operator<<(OutputStream& stream, unsigned long value);
	OutputStream& operator<<(OutputStream& stream, unsigned long long value);
}