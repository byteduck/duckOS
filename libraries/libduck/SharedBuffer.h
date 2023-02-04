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

#include <sys/shm.h>
#include <memory>
#include "Result.h"
#include "Object.h"
#include "Serializable.h"

namespace Duck {
	class SharedBuffer: public Duck::Object {
	public:
		DUCK_OBJECT_DEF(SharedBuffer);

		~SharedBuffer() noexcept;

		static ResultRet<Duck::Ptr<SharedBuffer>> alloc(size_t size);
		static ResultRet<Duck::Ptr<SharedBuffer>> adopt(int id);

		[[nodiscard]] ResultRet<Duck::Ptr<SharedBuffer>> copy() const;
		int allow(int pid, bool read = true, bool write = true);

		[[nodiscard]] void* ptr() const { return m_shm.ptr; }
		[[nodiscard]] size_t size() const { return m_shm.size; }
		[[nodiscard]] int id() const { return m_shm.id; }

		template<typename T>
		[[nodiscard]] T* ptr() const { return (T*) m_shm.ptr; }
		template<typename T>
		[[nodiscard]] size_t size() const { return size() / sizeof(T); }

	private:
		explicit SharedBuffer(struct shm shm_info);

		struct shm m_shm = {nullptr, 0, 0};
	};
}


