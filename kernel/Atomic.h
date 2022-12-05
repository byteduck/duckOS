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

	Copyright (c) Byteduck 2016-2021. All rights reserved.
*/

#pragma once

enum class MemoryOrder {
	Relaxed = __ATOMIC_RELAXED,
	Acquire = __ATOMIC_ACQUIRE,
	Release = __ATOMIC_RELEASE,
	AcqRel = __ATOMIC_ACQ_REL,
	SeqCst = __ATOMIC_SEQ_CST
};

template<typename T, MemoryOrder default_order = MemoryOrder::SeqCst>
class Atomic {
public:
	Atomic() = default;
	Atomic(T value): m_val(value) {}

	inline T load(MemoryOrder order = default_order) const volatile noexcept {
		return __atomic_load_n(&m_val, (int) order);
	}

	inline void store(T val, MemoryOrder order = default_order) volatile noexcept {
		__atomic_store_n(&m_val, val, (int) order);
	}

	inline T add(T val, MemoryOrder order = default_order) volatile noexcept {
		return __atomic_fetch_add(&m_val, val, (int) order);
	}

	inline T sub(T val, MemoryOrder order = default_order) volatile noexcept {
		return __atomic_fetch_sub(&m_val, val, (int) order);
	}

	inline bool compare_exchange_strong(T& expected, T desired, MemoryOrder order = default_order) volatile noexcept {
		if (order == MemoryOrder::AcqRel || order == MemoryOrder::Release)
			return __atomic_compare_exchange(&m_val, &expected, &desired, false, (int) MemoryOrder::Release, (int) MemoryOrder::Acquire);
		return __atomic_compare_exchange(&m_val, &expected, &desired, false, (int) order, (int) order);
	}

private:
	T m_val;
};


