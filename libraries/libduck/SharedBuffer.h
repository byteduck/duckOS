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

namespace Duck {
	class SharedBuffer {
	public:
		static ResultRet<SharedBuffer> create(size_t size);
		static ResultRet<SharedBuffer> attach(int id);

		explicit SharedBuffer(struct shm shm_info);

		[[nodiscard]] ResultRet<SharedBuffer> copy() const;
		int allow(int pid, bool read = true, bool write = true);
		[[nodiscard]] void* ptr() const;
		[[nodiscard]] size_t size() const;
		[[nodiscard]] int id() const;

	private:
		class ShmRef {
		public:
			~ShmRef() { shmdetach(shm_info.id); }
			struct shm shm_info;
		};

		std::shared_ptr<ShmRef> m_shm;
	};
}


