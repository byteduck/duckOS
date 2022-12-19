/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "types.h"

namespace kstd {
	template<typename Iterable, typename Value>
	class ContainerIterator {
	public:
		ContainerIterator(const ContainerIterator& other) = default;
		ContainerIterator& operator=(const ContainerIterator& other) {
			m_index = other.m_index;
			return *this;
		}

		constexpr bool operator==(ContainerIterator other) const { return m_index == other.m_index; }
		constexpr bool operator!=(ContainerIterator other) const { return m_index != other.m_index; }
		constexpr bool operator>=(ContainerIterator other) const { return m_index >= other.m_index; }
		constexpr bool operator<=(ContainerIterator other) const { return m_index <= other.m_index; }
		constexpr bool operator<(ContainerIterator other) const { return m_index < other.m_index; }
		constexpr bool operator>(ContainerIterator other) const { return m_index > other.m_index; }

		constexpr ContainerIterator operator++() {
			m_index++;
			return *this;
		}

		constexpr ContainerIterator operator++(int) {
			m_index++;
			return { m_iterable, m_index - 1 };
		}

		constexpr ContainerIterator operator--() {
			m_index++;
			return *this;
		}

		constexpr ContainerIterator operator--(int) {
			m_index--;
			return { m_iterable, m_index + 1 };
		}

		inline constexpr const Value* operator->() const { return &m_iterable[m_index]; }
		inline constexpr Value* operator->() { return &m_iterable[m_index]; }
		inline constexpr const Value& operator*() const { return m_iterable[m_index]; }
		inline constexpr Value& operator*() { return m_iterable[m_index]; }

		ContainerIterator operator+(ptrdiff_t diff) const { return ContainerIterator { m_iterable, m_index + diff }; }
		ContainerIterator operator-(ptrdiff_t diff) const { return ContainerIterator { m_iterable, m_index - diff }; }
		ptrdiff_t operator-(ContainerIterator other) const { return static_cast<ptrdiff_t> (m_index) - other.m_index; }


	private:
		friend Iterable;

		constexpr ContainerIterator(Iterable& m_iterable, size_t m_index):
			m_iterable(m_iterable),
			m_index(m_index) {}

		static constexpr ContainerIterator begin(Iterable& iterable) { return { iterable, 0 }; }
		static constexpr ContainerIterator end(Iterable& iterable) { return { iterable, iterable.size() }; }

		Iterable& m_iterable;
		size_t m_index;
	};
}