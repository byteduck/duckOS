/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include "../api/types.h"

class WriteableBytes;

class ReadableBytes {
public:
	ReadableBytes(uint8_t* ptr, size_t size): m_ptr(ptr), m_size(size) {};
	virtual ~ReadableBytes() = default;

	[[nodiscard]] virtual size_t size() const { return m_size; }
	[[nodiscard]] inline uint8_t* ptr() { return m_ptr; }
	[[nodiscard]] inline const uint8_t* ptr() const { return m_ptr; }

	void read(void* dest, size_t count, size_t start = 0) const;
	void read(WriteableBytes& dest, size_t count, size_t start = 0) const;

protected:

	uint8_t* m_ptr;
	size_t m_size;
};

class WriteableBytes: public ReadableBytes {
public:
	~WriteableBytes() override = default;
	WriteableBytes(uint8_t* ptr, size_t size): ReadableBytes(ptr, size) {}

	void write(const void* src, size_t count, size_t start = 0) const;
	void write(ReadableBytes& src, size_t count, size_t start = 0) const;
};