/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "../../Atomic.h"
#include "../utility.h"
#include "../kstdio.h"

namespace kstd {
	enum class PtrReleaseAction {
		Keep, Destroy
	};

	enum class SharedPtrType {
		Strong, Weak
	};

	class RefCount {
	public:
		explicit RefCount(int strong_count):
			m_strong_count(strong_count),
			m_weak_count(0) {}
		RefCount(const RefCount& other) = delete;
		RefCount(RefCount&& other):
			m_strong_count(other.m_strong_count),
			m_weak_count(other.m_weak_count) {}

		/**
		 * Gets the number of strong references.
		 */
		long strong_count() const {
			return m_strong_count.load();
		}

		/**
		 * Gets the number of weak references.
		 */
		long weak_count() const {
			return m_weak_count.load();
		}

		/**
		 * Tries to (safely) increase the strong reference count when
		 * we're not sure if it's at least 1.
		 * @return Whether a strong reference could be acquired.
		 */
		bool make_strong() {
			long expected = m_strong_count.load();
			if(!expected)
				return false;
			while(!m_strong_count.compare_exchange_strong(expected, expected + 1)) {
				if(!expected)
					return false;
			}
			return true;
		}

		/**
		 * Increases the strong reference count when we know that
		 * it's already at least one.
		 */
		void acquire_strong() {
			m_strong_count.add(1);
		}

		/**
		 * Acquires a weak reference.
		 */
		void acquire_weak() {
			m_weak_count.add(1);
		}

		/**
		 * Releases a strong reference.
		 * @return An action describing if the object should be destroyed or kept.
		 */
		PtrReleaseAction release_strong() {
			if(m_strong_count.sub(1) == 1) {
				if(m_weak_count.load() == 0)
					delete this;
				return PtrReleaseAction::Destroy;
			}
			return PtrReleaseAction::Keep;
		}

		void release_weak() {
			if(m_weak_count.sub(1) == 1) {
				if(m_strong_count.load() == 0)
					delete this;
			}
		}

	private:
		Atomic<long, MemoryOrder::AcqRel> m_strong_count = 0;
		Atomic<long, MemoryOrder::AcqRel> m_weak_count = 0;
	};
}
