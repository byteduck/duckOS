/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include <sys/mman.h>
#include "Buffer.h"
#include "Result.h"
#include "File.h"
#include "Log.h"

namespace Duck {
	class MappedBuffer: public Buffer {
	public:
		DUCK_OBJECT_DEF(MappedBuffer);

		enum Prot {
			R = PROT_READ,
			RW = PROT_READ | PROT_WRITE,
			RX = PROT_READ | PROT_EXEC,
			RWX = PROT_READ | PROT_WRITE | PROT_EXEC
		};

		enum Type {
			SharedFile = MAP_SHARED,
			PrivateFile = MAP_PRIVATE
		};

		~MappedBuffer() override;

		static ResultRet<Ptr<MappedBuffer>> make_file(const File& file, Prot prot, Type type, off_t offset = 0, size_t size = 0);
		static ResultRet<Ptr<MappedBuffer>> make_anonymous(Prot prot, size_t size);

		[[nodiscard]] size_t size() const override { return m_size; }
		[[nodiscard]] void* data() const override { return m_ptr; }

		/**
		 * Gets a pointer to the memory in the buffer.
		 * @tparam T The type of the pointer.
		 * @return The pointer.
		 */
		template<typename T>
		[[nodiscard]] T* data() const {
			return (T*) data();
		}

		/**
		 * Gets the size, in terms of a type, of the buffer.
		 * @tparam T The type to get the size in terms of.
		 * @return The size, in Ts, of the buffer.
		 */
		template<typename T>
		[[nodiscard]] size_t size() const {
			return size() / sizeof(T);
		}

	private:
		MappedBuffer(void *ptr, size_t size);

		void   *m_ptr;
		size_t  m_size;
	};
}
