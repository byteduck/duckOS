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
#pragma once

#include <memory>

namespace Duck {
	class ByteBuffer {
	public:
		/**
		 * Allocates a new ByteBuffer. Copies will point to the same memory - use clone() to create a copy.
		 * @param size The size, in bytes, of the buffer.
		 */
		explicit ByteBuffer(size_t size);

		/**
		 * Creates a new ByteBuffer that points to existing memory.
		 * @param ptr The pointer to memory.
		 * @param size The size, in bytes, of the buffer.
		 * @return The new ByteBuffer.
		 */
		static ByteBuffer adopt(void* ptr, size_t size);

		/**
		 * Creates a new ByteBuffer from a copy of existing memory.
		 * @param ptr The pointer to memory.
		 * @param size The size, in bytes, of the buffer.
		 * @return The new ByteBuffer.
		 */
		static ByteBuffer copy(const void* ptr, size_t size);

		/**
		 * Creates a new ByteBuffer from existing memory, which will not be freed on destruction.
		 * @param ptr The pointer to memory.
		 * @param size The size, in bytes, of the buffer.
		 * @return The new ByteBuffer.
		 */
		static ByteBuffer shadow(void* ptr, size_t size);

		/**
		 * Creates a clone of the ByteBuffer.
		 * @return A new, cloned ByteBuffer.
		 */
		[[nodiscard]] ByteBuffer clone() const;

		/**
		 * Gets a pointer to the memory in the buffer.
		 * @tparam T The type of the pointer.
		 * @return The pointer.
		 */
		template<typename T>
		[[nodiscard]] T* data() const {
			return (T*) m_ptr->ptr;
		}

		/**
		 * Gets the size of the buffer.
		 * @return The size, in bytes, of the buffer.
		 */
		[[nodiscard]] size_t size() const;

		/**
		 * Gets the size, in terms of a type, of the buffer.
		 * @tparam T The type to get the size in terms of.
		 * @return The size, in Ts, of the buffer.
		 */
		 template<typename T>
		[[nodiscard]] size_t size() const {
			return m_size / sizeof(T);
		}

	private:
		explicit ByteBuffer(void* ptr, size_t size);

		class BufferRef {
		public:
			explicit BufferRef(void* ptr);
			~BufferRef();
			void* ptr;
			bool free_on_destroy = true;
		};

		std::shared_ptr<BufferRef> m_ptr;
		size_t m_size;
	};
}


