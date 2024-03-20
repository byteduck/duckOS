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

#include "Stream.h"
#include "FileStream.h"
#include "FormatStream.h"
#include <kernel/api/ipv4.h>
#include <kernel/api/net.h>

using namespace Duck;

[[maybe_unused]] InputStream& Stream::std_in = FileStream::std_in;
[[maybe_unused]] OutputStream& Stream::std_out = FileStream::std_out;
[[maybe_unused]] OutputStream& Stream::std_err = FileStream::std_err;

Stream::~Stream() = default;

[[nodiscard]] Result Stream::status(){
	auto ret = m_err;
	m_err = 0;
	return ret;
}


/*
 * Stream operators
 */
namespace Duck {
	/*
	 * InputStream stuff
	 */

	InputStream& operator>>(InputStream& stream, uint8_t& byte) {
		stream.read(&byte, 1);
		return stream;
	}

	InputStream& operator>>(InputStream& stream, char& chr) {
		stream.read(&chr, 1);
		return stream;
	}

	InputStream& operator>>(InputStream& stream, std::string& string) {
		string = "";
		char read;
		size_t nread = stream.read(&read, 1);

		while(read != stream.delimeter() && nread) {
			string += read;
			nread = stream.read(&read, 1);
		}

		return stream;
	}

	/*
	 * OutputStream strings
	 */

	OutputStream& operator<<(OutputStream& stream, const char* chars) {
		stream.write(chars, strlen(chars));
		return stream;
	}

	OutputStream& operator<<(OutputStream& stream, const std::string& string) {
		stream.write(string.c_str(), string.size());
		return stream;
	}

	OutputStream& operator<<(OutputStream& stream, const std::string_view& view) {
		stream.write(view.data(), view.size());
		return stream;
	}

	/*
	 * OutputStream Results
	 */
	OutputStream& operator<<(OutputStream& stream, const Result& result) {
		if (result.is_error())
			return stream << result.message();
		else
			return stream << "Success";
	}

	/*
	 * OutputStream primitives
	 */

	OutputStream& operator<<(OutputStream& stream, char chr) {
		stream.write(&chr, 1);
		return stream;
	}

	OutputStream& operator<<(OutputStream& stream, const uint8_t& byte) {
		stream.write(&byte, 1);
		return stream;
	}

	OutputStream& operator<<(OutputStream& stream, int value) {
		return operator<<(stream, std::to_string(value));
	}

	OutputStream& operator<<(OutputStream& stream, long value) {
		return operator<<(stream, std::to_string(value));
	}

	OutputStream& operator<<(OutputStream& stream, float value) {
		return operator<<(stream, std::to_string(value));
	}

	OutputStream& operator<<(OutputStream& stream, double value) {
		return operator<<(stream, std::to_string(value));
	}

	OutputStream& operator<<(OutputStream& stream, long long value) {
		return operator<<(stream, std::to_string(value));
	}

	OutputStream& operator<<(OutputStream& stream, long double value) {
		return operator<<(stream, std::to_string(value));
	}

	OutputStream& operator<<(OutputStream& stream, unsigned int value) {
		return operator<<(stream, std::to_string(value));
	}

	OutputStream& operator<<(OutputStream& stream, unsigned long value) {
		return operator<<(stream, std::to_string(value));
	}

	OutputStream& operator<<(OutputStream& stream, unsigned long long value) {
		return operator<<(stream, std::to_string(value));
	}
}

/*
 * OutputStream misc
 */
OutputStream& operator<<(OutputStream& stream, const IPv4Address& address) {
	stream % "{}.{}.{}.{}" % (int) address[0] % (int) address[1] % (int) address[2] % (int) address[3];
	return stream;
}

OutputStream& operator<<(OutputStream& stream, const MACAddress& address) {
	stream % "{x}:{x}:{x}:{x}:{x}:{x}" % (int) address[0] % (int) address[1] % (int) address[2] % (int) address[3] % (int) address[4] % (int) address[5];
	return stream;
}