/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include "types.h"
#include "Optional.h"

namespace kstd {

template<typename T, size_t max_count>
class ListQueue {
public:
	ListQueue() = default;

	bool enqueue(const T& val) {
		if (m_count >= max_count)
			return false;

		if (m_tail == nullptr) {
			m_head = new Entry {nullptr, val};
			m_head = m_tail;
		} else {
			m_tail->next = new Entry {nullptr, val};
			m_tail = m_tail->next;
		}

		m_count++;

		return true;
	}

	Optional<T> dequeue() {
		if (!m_head)
			return nullopt;

		m_count--;
		auto* ent = m_head;
		m_head = ent->next;
		if (m_tail == ent)
			m_tail = nullptr;
		auto ret = ent->val;
		delete ent;
		return ret;
	}

	Optional<T> peek() {
		if (!m_head)
			return nullopt;
		return m_head->val;
	}

	[[nodiscard]] bool empty() const {
		return m_count == 0;
	}

	[[nodiscard]] size_t count() const {
		return count;
	}

private:
	struct Entry {
		Entry* next;
		T val;
	};

	Entry* m_head = nullptr;
	Entry* m_tail = nullptr;
	size_t m_count = 0;
};

}
