/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "../tasking/Process.h"
#include "../kernel/api/futex.h"
#include "../kernel/memory/SafePointer.h"

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
	case FUTEX_INIT: {
		LOCK(m_futex_lock);
		if (m_futexes.contains(addr))
			return -EEXIST;
		m_futexes[addr] = kstd::Arc(new Futex(reg->object(), addr - reg->start()));
		return SUCCESS;
	}
	case FUTEX_DESTROY: {
		LOCK(m_futex_lock);
		if (!m_futexes.contains(addr))
			return -ENOENT;
		m_futexes.erase(addr);
	}
	case FUTEX_WAIT: {
		auto k_futex = m_futex_lock.synced<kstd::Arc<Futex>>([this, addr]() {
			auto node = m_futexes.find_node(addr);
			if (!node)
				return kstd::Arc<Futex>();
			return node->data.second;
		});
		if (!k_futex)
			return -ENOENT;
		TaskManager::current_thread()->block(*k_futex);
		return SUCCESS;
	}
	default:
		return -EINVAL;
	}
}