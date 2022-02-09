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

#include "SharedBuffer.h"
#include "Log.h"
using namespace Duck;

ResultRet<SharedBuffer> SharedBuffer::create(size_t size) {
	shm shm_info;
	if(shmcreate(nullptr, size, &shm_info))
		return Result(errno);
	return SharedBuffer {shm_info};
}

ResultRet<SharedBuffer> SharedBuffer::attach(int id) {
	shm shm_info;
	if(shmattach(id, nullptr, &shm_info))
		return Result(errno);
	return SharedBuffer {shm_info};
}

SharedBuffer::SharedBuffer(shm shm_info): m_shm(std::shared_ptr<ShmRef>(new ShmRef {shm_info})) {

}

ResultRet<SharedBuffer> SharedBuffer::copy() {
	auto cpy_res = create(m_shm->shm_info.size);
	if(cpy_res.is_error())
		return cpy_res.result();
	memcpy(cpy_res.value().ptr(), ptr(), m_shm->shm_info.size);
	return std::move(cpy_res.value());
}

int SharedBuffer::allow(int pid, bool read, bool write) {
	return shmallow(m_shm->shm_info.id, pid, (read ? SHM_READ : 0) | (write ? SHM_WRITE : 0));
}

void* SharedBuffer::ptr() const {
	return m_shm->shm_info.ptr;
}

size_t SharedBuffer::size() const {
	return m_shm->shm_info.size;
}

int SharedBuffer::id() const {
	return m_shm->shm_info.id;
}
