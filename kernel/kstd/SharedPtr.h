/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "bits/RefCount.h"

namespace kstd {
	template<typename T>
	class WeakPtr;

	template<typename T>
	class Shared;
	class SharedBase;

	template<typename T>
	void __set_shared_weak_self(SharedBase* base, WeakPtr<T> weak);

	template<typename T>
	class SharedPtr {
	public:
		SharedPtr(): m_ptr(nullptr), m_count(nullptr) {}

		// SharedPtr<U> -> SharedPtr<T>
		template<typename U>
		SharedPtr(const SharedPtr<U>& other):
			m_ptr((T*)(other.m_ptr)),
			m_count(other.m_count)
		{
			if(m_count)
				m_count->acquire_strong();
		}

		// Copy constructor
		SharedPtr(const SharedPtr<T>& other):
				m_ptr(other.m_ptr),
				m_count(other.m_count)
		{
			if(m_count)
				m_count->acquire_strong();
		}

		// Move SharedPtr<U> -> SharedPtr<T>
		template<typename U>
		SharedPtr(SharedPtr<U>&& other):
			m_ptr((T*)(other.m_ptr)),
			m_count(other.m_count)
		{
			other.m_ptr = nullptr;
			other.m_count = nullptr;
		}

		// Move constructor
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
		{
			if constexpr(is_base_of<SharedBase, T>) {
				if(ptr)
					__set_shared_weak_self(static_cast<SharedBase*>(ptr), WeakPtr<T>(*this));
			}
		}

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

		void leak_ref() {
			if(m_count)
				m_count->acquire_strong();
		}

		void leak_unref() {
			if(m_count)
				m_count->release_strong();
		}

		SharedPtr<T>& operator=(SharedPtr<T> other) {
			kstd::swap(m_ptr, other.m_ptr);
			kstd::swap(m_count, other.m_count);
			return *this;
		}

		RefCount* ref_count() const {
			return m_count;
		}

		void reset() {
			if(m_count) {
				if(m_count->release_strong() == PtrReleaseAction::Destroy)
					delete m_ptr;
			}
			m_ptr = nullptr;
			m_count = nullptr;
		}

		template<typename U>
		bool operator==(const SharedPtr<U>& other) const {
			return other.m_ptr == m_ptr;
		}

		template<typename U>
		bool operator!=(const SharedPtr<U>& other) const {
			return other.m_ptr != m_ptr;
		}

	private:
		template<typename U>
		friend class SharedPtr;
		template<typename U>
		friend class WeakPtr;

		T* m_ptr = nullptr;
		RefCount* m_count = nullptr;
	};

	template<typename T>
	class WeakPtr {
	public:
		WeakPtr(): m_ptr(nullptr), m_count(nullptr) {}

		// SharedPtr<U> -> WeakPtr<T>
		template<typename U>
		WeakPtr(const SharedPtr<U>& other):
				m_ptr((T*)(other.m_ptr)),
				m_count(other.m_count)
		{
			if(m_count)
				m_count->acquire_weak();
		}

		// Move SharedPtr<U> -> WeakPtr<T>
		template<typename U>
		WeakPtr(SharedPtr<U>&& other):
				m_ptr((T*)(other.m_ptr)),
				m_count(other.m_count)
		{
			if(m_count)
				m_count->acquire_weak();
		}

		// WeakPtr<U> -> WeakPtr<T>
		template<typename U>
		WeakPtr(const WeakPtr<U>& other):
				m_ptr((T*)(other.m_ptr)),
				m_count(other.m_count)
		{
			if(m_count)
				m_count->acquire_weak();
		}

		// Copy constructor
		WeakPtr(const WeakPtr<T>& other):
				m_ptr(other.m_ptr),
				m_count(other.m_count)
		{
			if(m_count)
				m_count->acquire_weak();
		}

		// Move WeakPtr<U> -> WeakPtr<T>
		template<typename U>
		WeakPtr(WeakPtr<U>&& other):
				m_ptr(static_cast<T*>(other.m_ptr)),
				m_count(other.m_count)
		{
			other.m_ptr = nullptr;
			other.m_count = nullptr;
		}

		// Move constructor
		WeakPtr(WeakPtr<T>&& other):
				m_ptr(other.m_ptr),
				m_count(other.m_count)
		{
			other.m_ptr = nullptr;
			other.m_count = nullptr;
		}

		~WeakPtr() {
			reset();
		}

		SharedPtr<T> lock() {
			if(m_count && m_count->make_strong()) {
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

		RefCount* ref_count() const {
			return m_count;
		}

		void reset() {
			if(m_count)
				m_count->release_weak();
			m_ptr = nullptr;
			m_count = nullptr;
		}

		template<typename F>
		void with_locked(F&& func) {
			auto locked_self = lock();
			if(locked_self)
				func(locked_self);
		}

		template<typename U>
		bool operator==(const WeakPtr<U>& other) {
			return other.m_ptr == m_ptr;
		}

		template<typename U>
		bool operator!=(const WeakPtr<U>& other) {
			return other.m_ptr != m_ptr;
		}

	private:
		template<typename U>
		friend class WeakPtr;

		T* m_ptr = nullptr;
		RefCount* m_count = nullptr;
	};

	template<typename T, class... Args>
	SharedPtr<T> make_shared(Args&&... args) {
		return SharedPtr<T>(new T(args...));
	}

	template<class T, class U>
	SharedPtr<T> static_pointer_cast(const SharedPtr<U>& ptr) // never throws
	{
		return SharedPtr<T>(static_cast<T*>(ptr.get()), ptr.ref_count());
	}

	template<class T, class U>
	WeakPtr<T> static_pointer_cast(const WeakPtr<U>& ptr) // never throws
	{
		return WeakPtr<T>(static_cast<T*>(ptr.get()), ptr.ref_count());
	}

	class SharedBase {
	protected:
		template<typename T>
		friend void __set_shared_weak_self(SharedBase* base, WeakPtr<T> weak);

		WeakPtr<void*> m_weak_self;
	};

	template<typename T>
	class Shared: public SharedBase {
	public:
		inline SharedPtr<T> self() {
			ASSERT(m_weak_self.operator bool());
			return m_weak_self.lock();
		}
	};

	template<typename T>
	void __set_shared_weak_self(SharedBase* base, WeakPtr<T> weak) {
		base->m_weak_self = weak;
	}

	template<typename T>
	using shared_ptr = SharedPtr<T>;
}

template<typename T>
using Ptr = kstd::SharedPtr<T>;

template<typename T>
using Weak = kstd::WeakPtr<T>;