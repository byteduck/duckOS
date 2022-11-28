/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "bits/SharedPtrBase.h"

namespace kstd {
	template<typename T>
	class SharedPtr {
	public:
		SharedPtr(): m_ptr(nullptr), m_count(nullptr) {}

		SharedPtr(const SharedPtr<T>& other):
			m_ptr(other.m_ptr),
			m_count(other.m_count)
		{
			if(m_count)
				m_count->acquire_strong();
		}

		SharedPtr(SharedPtr<T>&& other):
			m_ptr(other.m_ptr),
			m_count(other.m_count)
		{
			other.m_ptr = nullptr;
			other.m_count = nullptr;
		}

		explicit SharedPtr(T* ptr):
			m_ptr(ptr),
			m_count(ptr ? new RefCount(1) : nullptr)
		{}

		explicit SharedPtr(T* ptr, RefCount* count):
			m_ptr(ptr),
			m_count(count)
		{
			if(m_count)
				m_count->acquire_strong();
		}

		~SharedPtr() {
			reset();
		}

		T* get() const {
			return m_ptr;
		}

		T& operator*() const {
			ASSERT(m_ptr);
			return *m_ptr;
		}

		T* operator->() const {
			ASSERT(m_ptr);
			return m_ptr;
		}

		operator bool() const {
			return m_count && m_count->strong_count();
		}

		SharedPtr<T>& operator=(SharedPtr<T> other) {
			kstd::swap(m_ptr, other.m_ptr);
			kstd::swap(m_count, other.m_count);
			return *this;
		}

		void reset() {
			if(m_count) {
				if(m_count->release_strong() == PtrReleaseAction::Destroy)
					delete m_ptr;
			}
			m_ptr = nullptr;
			m_count = nullptr;
		}

	private:
		T* m_ptr = nullptr;
		RefCount* m_count = nullptr;
	};

	template<typename T>
	class WeakPtr {
	public:
		WeakPtr(): m_ptr(nullptr), m_count(nullptr) {}

		WeakPtr(const SharedPtr<T>& other):
				m_ptr(other.m_ptr),
				m_count(other.m_count)
		{
			if(m_count)
				m_count->acquire_strong();
		}

		WeakPtr(SharedPtr<T>&& other):
				m_ptr(other.m_ptr),
				m_count(other.m_count)
		{
			other.m_ptr = nullptr;
			other.m_count = nullptr;
		}

		explicit WeakPtr(T* ptr):
				m_ptr(ptr),
				m_count(new RefCount(1))
		{}

		~WeakPtr() {
			reset();
		}

		SharedPtr<T> lock() {
			if(m_count->make_strong()) {
				auto ret = SharedPtr<T>(m_ptr, m_count);
				m_count->release_strong();
				return ret;
			}
			return SharedPtr<T>(nullptr);
		}

		operator bool() const {
			return m_count && m_count->strong_count();
		}

		WeakPtr<T>& operator=(WeakPtr<T> other) {
			kstd::swap(m_ptr, other.m_ptr);
			kstd::swap(m_count, other.m_count);
			return *this;
		}

		void reset() {
			if(m_count)
				m_count->release_weak();
			m_ptr = nullptr;
			m_count = nullptr;
		}

	private:
		T* m_ptr = nullptr;
		RefCount* m_count = nullptr;
	};
}