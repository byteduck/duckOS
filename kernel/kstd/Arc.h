/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "bits/RefCount.h"

namespace kstd {
	template<typename T>
	class Weak;

	template<typename T>
	class ArcSelf;
	class ArcSelfBase;

	template<typename T>
	void __set_shared_weak_self(ArcSelfBase* base, Weak<T> weak);

	template<typename T>
	class Arc {
	public:
		Arc(): m_ptr(nullptr), m_count(nullptr) {}

		// Arc<U> -> Arc<T>
		template<typename U>
		Arc(const Arc<U>& other):
			m_ptr(static_cast<T*>(other.m_ptr)),
			m_count(other.m_count)
		{
			if(m_count)
				m_count->acquire_strong();
		}

		// Copy constructor
		Arc(const Arc<T>& other):
				m_ptr(other.m_ptr),
				m_count(other.m_count)
		{
			if(m_count)
				m_count->acquire_strong();
		}

		// Move Arc<U> -> Arc<T>
		template<typename U>
		Arc(Arc<U>&& other) noexcept:
			m_ptr(static_cast<T*>(other.m_ptr)),
			m_count(other.m_count)
		{
			other.m_ptr = nullptr;
			other.m_count = nullptr;
		}

		// Move constructor
		Arc(Arc<T>&& other) noexcept:
				m_ptr(other.m_ptr),
				m_count(other.m_count)
		{
			other.m_ptr = nullptr;
			other.m_count = nullptr;
		}

		// Special case for ArcSelf weak -> strong conversion
		template<typename U = void*>
		Arc(Arc<void*>&& other) noexcept:
				m_ptr((T*)(other.m_ptr)),
				m_count(other.m_count)
		{
			other.m_ptr = nullptr;
			other.m_count = nullptr;
		}

		explicit Arc(T* ptr):
			m_ptr(ptr),
			m_count(ptr ? new RefCount(1) : nullptr)
		{
			if constexpr(is_base_of<ArcSelfBase, T>) {
				if(ptr)
					__set_shared_weak_self(static_cast<ArcSelfBase*>(ptr), Weak<T>(*this));
			}
		}

		explicit Arc(T* ptr, RefCount* count):
			m_ptr(ptr),
			m_count(count)
		{
			if(m_count)
				m_count->acquire_strong();
		}

		template<typename... Args>
		static Arc<T> make(Args&&... args) {
			return Arc<T>(new T(args...));
		}

		~Arc() {
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

		Arc<T>& operator=(const Arc<T>& other) noexcept {
			if(&other == this)
				return *this;
			reset();
			m_ptr = other.m_ptr;
			m_count = other.m_count;
			if(m_count)
				m_count->acquire_strong();
			return *this;
		}

		Arc<T>& operator=(Arc<T>&& other) noexcept {
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
		bool operator==(const Arc<U>& other) const {
			return other.m_ptr == m_ptr;
		}

		template<typename U>
		bool operator!=(const Arc<U>& other) const {
			return other.m_ptr != m_ptr;
		}

	private:
		template<typename U>
		friend class Arc;
		template<typename U>
		friend class Weak;

		T* m_ptr = nullptr;
		RefCount* m_count = nullptr;
	};

	template<typename T>
	class Weak {
	public:
		Weak(): m_ptr(nullptr), m_count(nullptr) {}

		// Arc<U> -> Weak<T>
		template<typename U>
		Weak(const Arc<U>& other):
				m_ptr(static_cast<T*>(other.m_ptr)),
				m_count(other.m_count)
		{
			if(m_count)
				m_count->acquire_weak();
		}

		// Move Arc<U> -> Weak<T>
		template<typename U>
		Weak(Arc<U>&& other):
				m_ptr(static_cast<T*>(other.m_ptr)),
				m_count(other.m_count)
		{
			if(m_count)
				m_count->acquire_weak();
		}

		// Weak<U> -> Weak<T>
		template<typename U>
		Weak(const Weak<U>& other):
				m_ptr((T*)(other.m_ptr)),
				m_count(other.m_count)
		{
			if(m_count)
				m_count->acquire_weak();
		}

		// Copy constructor
		Weak(const Weak<T>& other):
				m_ptr(other.m_ptr),
				m_count(other.m_count)
		{
			if(m_count)
				m_count->acquire_weak();
		}

		// Move Weak<U> -> Weak<T>
		template<typename U>
		Weak(Weak<U>&& other):
				m_ptr(static_cast<T*>(other.m_ptr)),
				m_count(other.m_count)
		{
			other.m_ptr = nullptr;
			other.m_count = nullptr;
		}

		// Move constructor
		Weak(Weak<T>&& other):
				m_ptr(other.m_ptr),
				m_count(other.m_count)
		{
			other.m_ptr = nullptr;
			other.m_count = nullptr;
		}

		~Weak() {
			reset();
		}

		Arc<T> lock() const {
			if(m_count && m_count->make_strong()) {
				auto ret = Arc<T>(m_ptr, m_count);
				m_count->release_strong();
				return ret;
			}
			return Arc<T>(nullptr);
		}

		operator bool() const {
			return m_count && m_count->strong_count();
		}

		Weak<T>& operator=(const Weak<T>& other) noexcept {
			if(&other == this)
				return *this;
			reset();
			m_ptr = other.m_ptr;
			m_count = other.m_count;
			if(m_count)
				m_count->acquire_weak();
			return *this;
		}

		Weak<T>& operator=(Weak<T>&& other) noexcept {
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
		bool operator==(const Weak<U>& other) {
			return other.m_ptr == m_ptr;
		}

		template<typename U>
		bool operator!=(const Weak<U>& other) {
			return other.m_ptr != m_ptr;
		}

	private:
		template<typename U>
		friend class Weak;

		T* m_ptr = nullptr;
		RefCount* m_count = nullptr;
	};

	template<typename T, class... Args>
	Arc<T> make_shared(Args&&... args) {
		return Arc<T>(new T(args...));
	}

	template<class T, class U>
	Arc<T> static_pointer_cast(const Arc<U>& ptr) // never throws
	{
		return Arc<T>(static_cast<T*>(ptr.get()), ptr.ref_count());
	}

	template<class T, class U>
	Weak<T> static_pointer_cast(const Weak<U>& ptr) // never throws
	{
		return Weak<T>(static_cast<T*>(ptr.get()), ptr.ref_count());
	}

	class ArcSelfBase {
	protected:
		template<typename T>
		friend void __set_shared_weak_self(ArcSelfBase* base, Weak<T> weak);

		Weak<void*> m_weak_self;
	};

	template<typename T>
	class ArcSelf: public ArcSelfBase {
	public:
		inline Arc<T> self() {
			ASSERT(m_weak_self.operator bool());
			return m_weak_self.lock();
		}
	};

	template<typename T>
	void __set_shared_weak_self(ArcSelfBase* base, Weak<T> weak) {
		base->m_weak_self = weak;
	}
}