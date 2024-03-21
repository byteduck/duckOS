/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "kstdio.h"
#include "utility.h"

namespace kstd {
	struct nullopt_t {};

	extern nullopt_t nullopt;

	template<typename T>
	class Optional {
	public:
		Optional(const T& value):
			m_value(new T(value))
		{}

		Optional(T&& value) noexcept:
			m_value(new T(value))
		{}

		Optional(nullopt_t):
			m_value(nullptr)
		{}

		Optional(const Optional<T>& other):
			m_value(other.m_value ? new T(*other.m_value) : nullptr)
		{}

		template<typename U>
		Optional(const Optional<U>& other):
				m_value(other.m_value ? new T(*static_cast<T*>(other.m_value)) : nullptr)
		{}

		Optional(Optional<T>&& other) noexcept:
				m_value(other.m_value)
		{
			other.m_value = nullptr;
		}

		template<typename U>
		Optional(Optional<U>&& other) noexcept:
				m_value(static_cast<T*>(other.m_value))
		{
			other.m_value = nullptr;
		}

		~Optional() {
			delete m_value;
		}

		[[nodiscard]] bool has_value() const {
			return m_value;
		}

		[[nodiscard]] operator bool() const {
			return m_value;
		}

		const T& value() const {
			ASSERT(m_value);
			return *m_value;
		}

		Optional<T>& operator =(const Optional<T>& other) {
			if(&other == this)
				return *this;
			m_value = new T(other.m_value);
		}

		Optional<T>& operator =(Optional<T>&& other) noexcept {
			kstd::swap(other.m_value, m_value);
			return *this;
		}

	private:
		T* m_value;
	};
}