/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "../tasking/Process.h"
#include "../api/futex.h"
#include "../memory/SafePointer.h"
#include "../filesystem/FileDescriptor.h"
#include "../tasking/Futex.h"

int Process::sys_futex(UserspacePointer<futex_t> futex, int op) {
	auto addr = (uintptr_t) futex.raw();
	if (addr > HIGHER_HALF)
		return -EFAULT;
	auto reg_res = _vm_space->get_region_containing(addr);
	if (reg_res.is_error())
		return -EFAULT;
	auto reg = reg_res.value();
	if (!reg->prot().read || !reg->prot().write)
		return -EPERM;

	switch (op) {
	case FUTEX_REGFD:
		return m_fd_lock.synced<int>([this, addr, reg] {
			auto futex = kstd::Arc(new Futex(reg->object(), addr - reg->start()));
			auto fd = kstd::Arc(new FileDescriptor(futex, this));
			_file_descriptors.push_back(fd);
			fd->set_id((int) _file_descriptors.size() - 1);
			return (int)_file_descriptors.size() - 1;
		});
	case FUTEX_WAIT: {
		Futex k_futex {reg->object(), addr - reg->start()};
		TaskManager::current_thread()->block(k_futex);
		return SUCCESS;
	}
	default:
		return -EINVAL;
	}
}