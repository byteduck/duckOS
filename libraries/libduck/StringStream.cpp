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

#include "StringStream.h"
using namespace Duck;

size_t StringInputStream::read(void* buffer, size_t n) {
	auto view = std::string_view(m_string).substr(m_offset);

	if(n > view.size())
		n = view.size();

	memcpy(buffer, view.data(), n);
	m_offset += n;
	m_eof = m_offset == m_string.length();

	return n;
}

Result StringInputStream::seek(long offset, Whence whence) {
	switch(whence) {
	case SET:
		m_offset = offset;
		break;
	case CUR:
		m_offset += offset;
		break;
	case END:
		m_offset = m_string.length() + offset;
		break;
	}

	m_eof = false;

	if(m_offset > m_string.length()) {
		m_offset = m_string.length();
		return Result(EINVAL);
	}

	return Result::SUCCESS;
}

size_t StringOutputStream::write(const void* buffer, size_t n) {
	if(m_offset + n > m_string.size())
		m_string.resize(m_offset + n, ' ');
	m_string.replace(m_offset, n, (const char*) buffer, n);
	m_offset += n;
	return n;
}

Result StringOutputStream::seek(long offset, Whence whence) {
	switch(whence) {
		case SET:
			m_offset = offset;
			break;
		case CUR:
			m_offset += offset;
			break;
		case END:
			m_offset = m_string.length() + offset;
			break;
	}

	if(m_offset > m_string.length()) {
		m_offset = m_string.length();
		return Result(EINVAL);
	}

	return Result::SUCCESS;
}
