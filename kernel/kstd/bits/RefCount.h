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

	class RefCount {
	public:
		explicit RefCount(int strong_count);
		RefCount(RefCount&& other);
		RefCount(const RefCount& other) = delete;
		~RefCount();

		/**
		 * Gets the number of strong references.
		 */
		long strong_count() const;

		/**
		 * Gets the number of weak references.
		 */
		long weak_count() const;

		/**
		 * Tries to (safely) increase the strong reference count when
		 * we're not sure if it's at least 1.
		 * @return Whether a strong reference could be acquired.
		 */
		bool make_strong();

		/**
		 * Increases the strong reference count when we know that
		 * it's already at least one.
		 */
		void acquire_strong();

		/**
		 * Acquires a weak reference.
		 */
		void acquire_weak();

		/**
		 * Releases a strong reference.
		 * @return An action describing if the object should be destroyed or kept.
		 */
		PtrReleaseAction release_strong();

		/**
		 * Releases a weak reference.
		 */
		void release_weak();

	private:
		Atomic<long, MemoryOrder::SeqCst> m_strong_count = 0;
		Atomic<long, MemoryOrder::SeqCst> m_weak_count = 0;
	};
}
