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
    
    Copyright (c) Byteduck 2016-2022. All rights reserved.
*/
#include <cstring>
#include "ByteBuffer.h"

using namespace Duck;

ByteBuffer::ByteBuffer(size_t size): m_ptr(new uint8_t[size]), m_size(size), m_free_on_destroy(true) {}
ByteBuffer::ByteBuffer(void* ptr, size_t size): m_ptr(ptr), m_size(size), m_free_on_destroy(true) {}

ByteBuffer::~ByteBuffer() noexcept {
	if (m_free_on_destroy)
		free(m_ptr);
}

Ptr<ByteBuffer> ByteBuffer::adopt(void* ptr, size_t size) {
	return make(ptr, size);
}

Ptr<ByteBuffer> ByteBuffer::copy(const void* ptr, size_t size) {
	auto buf = make(size);
	memcpy(buf->data<void>(), ptr, size);
	return buf;
}

Ptr<ByteBuffer> ByteBuffer::clone() const {
	auto buf = make(m_size);
	memcpy(buf->data<void>(), data<void>(), m_size);
	return buf;
}

Ptr<ByteBuffer> ByteBuffer::shadow(void* ptr, size_t size) {
	auto ret = make(ptr, size);
	ret->m_free_on_destroy = false;
	return ret;
}

void* ByteBuffer::data() const {
	return m_ptr;
}

size_t ByteBuffer::size() const {
	return m_size;
}