/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "Bytes.h"
#include "../kstd/kstdio.h"
#include "../kstd/cstring.h"

void ReadableBytes::read(void* dest, size_t count, size_t start) const {
	if ((start + count <= start) || (start + count > m_size)) {
		ASSERT(false);
		return;
	}
	memcpy(dest, m_ptr + start, count);
}

void ReadableBytes::read(WriteableBytes& dest, size_t count, size_t start) const {
	if (dest.size() < count) {
		ASSERT(false);
		return;
	}
	read(dest.m_ptr, count, start);
}

void WriteableBytes::write(const void* src, size_t count, size_t start) const {
	if ((start + count <= start) || (start + count > m_size)) {
		ASSERT(false);
		return;
	}
	memcpy(m_ptr + start, src, count);
}

void WriteableBytes::write(ReadableBytes& src, size_t count, size_t start) const {
	if (src.size() < count) {
		ASSERT(false);
		return;
	}
	write(src.ptr(), count, start);
}
