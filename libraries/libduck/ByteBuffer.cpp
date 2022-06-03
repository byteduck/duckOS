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

ByteBuffer::ByteBuffer(size_t size): m_ptr(std::make_shared<BufferRef>(malloc(size))), m_size(size) {}
ByteBuffer::ByteBuffer(void* ptr, size_t size): m_ptr(std::make_shared<BufferRef>(ptr)), m_size(size) {}

ByteBuffer ByteBuffer::adopt(void* ptr, size_t size) {
	return ByteBuffer(ptr, size);
}

ByteBuffer ByteBuffer::copy(const void* ptr, size_t size) {
	ByteBuffer buf = ByteBuffer(size);
	memcpy(buf.data<void>(), ptr, size);
	return buf;
}

ByteBuffer ByteBuffer::clone() const {
	auto buf = ByteBuffer(m_size);
	memcpy(buf.data<void>(), data<void>(), m_size);
	return buf;
}

ByteBuffer ByteBuffer::shadow(void* ptr, size_t size) {
	ByteBuffer ret(ptr, size);
	ret.m_ptr->free_on_destroy = false;
	return ret;
}

size_t ByteBuffer::size() const {
	return m_size;
}

ByteBuffer::BufferRef::BufferRef(void* ptr): ptr(ptr) {}
ByteBuffer::BufferRef::~BufferRef() {
	if(free_on_destroy)
		free(ptr);
}
