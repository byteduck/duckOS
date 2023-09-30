/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "MappedBuffer.h"
#include "Log.h"

using namespace Duck;

ResultRet<Ptr<MappedBuffer>> MappedBuffer::make_file(const File& file, Prot prot, Type type, off_t offset, size_t length) {
	size_t fsize = file.stat().st_size;
	void* ptr = mmap(nullptr, length ? length : fsize, prot, type, file.fd(), offset);
	if (ptr == MAP_FAILED)
		return Result(errno);
	return make(ptr, length ? length : fsize);
}

ResultRet<Ptr<MappedBuffer>> MappedBuffer::make_anonymous(MappedBuffer::Prot prot, size_t size) {
	void* ptr = mmap(nullptr, size, prot, MAP_ANONYMOUS, -1, 0);
	if (ptr == MAP_FAILED)
		return Result(errno);
	return make(ptr, size);
}

MappedBuffer::MappedBuffer(void* ptr, size_t size): m_ptr(ptr), m_size(size) {

}

MappedBuffer::~MappedBuffer() {
	if(munmap(m_ptr, m_size)) {
		Duck::Log::warnf("libduck: Couldnt't unmap mapped buffer at {#x} with size {#x}: {}", (size_t) m_ptr, m_size, strerror(errno));
	}
}
