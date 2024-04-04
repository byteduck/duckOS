/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include <kernel/tasking/TaskManager.h>
#include "MemoryManager.h"

template<typename T>
class SafePointer {
public:
	SafePointer() = default;
	explicit SafePointer(T* raw_ptr, bool is_user):
		m_ptr(raw_ptr), m_is_user(is_user) {}
	template<typename C> SafePointer(const SafePointer<C>& safe_ptr):
		m_ptr((T*) safe_ptr.raw()), m_is_user(safe_ptr.is_user()) {}

	T* raw() const { return m_ptr; }
	bool is_user() const { return m_is_user; }

	/**
	 * Copies the value pointed to by this pointer from user memory.
	 * @param index An optional index into this pointer as an array.
	 * @return The value copied from userspace at the specified index.
	 */
	// TODO: This is gonna have to be a result. Right now, if we fail this check, we send SIGSEGV but since we're probably in a syscall, nothing will happen yet.
	inline T get(int index = 0) const {
		return checked<T>(false, index, 1, [&]() {
			return m_ptr[index];
		});
	}

	/**
	 * Sets the value pointed to by this pointer in user memory.
	 * @param index An index into this pointer as an array.
	 */
	// TODO: This is gonna have to be a result. Right now, if we fail this check, we send SIGSEGV but since we're probably in a syscall, nothing will happen yet.
	inline void set(int index, const T& value) const {
		checked<void>(true, index, 1, [&]() {
			m_ptr[index] = value;
		});
	}

	/**
	 * Sets the value pointed to by this pointer to the given value.
	 * @param value The value to set.
	 */
	// TODO: This is gonna have to be a result. Right now, if we fail this check, we send SIGSEGV but since we're probably in a syscall, nothing will happen yet.
	inline void set(const T& value) const {
		set(0, value);
	}

	/**
	 * Casts this pointer to a char pointer, and then safely makes a string from it.
	 * @return A string made from the C string that this pointer points to.
	 */
	// TODO: This is gonna have to be a result. Right now, if we fail this check, we send SIGSEGV but since we're probably in a syscall, nothing will happen yet.
	kstd::string str() const {
		// If this is a kernel pointer, don't check
		if(!m_is_user)
			return kstd::string((char*) m_ptr);

		auto* proc = TaskManager::current_thread()->process();
		return proc->vm_space()->lock().synced<kstd::string>([&]() {
			auto cur_ptr = (char*) m_ptr;
			proc->check_ptr(cur_ptr, false);
			auto last_checked_page = (size_t) cur_ptr / PAGE_SIZE;
			do {
				if((size_t) cur_ptr / PAGE_SIZE != last_checked_page) {
					proc->check_ptr(cur_ptr, false);
					last_checked_page = (size_t) cur_ptr / PAGE_SIZE;
				}
				cur_ptr++;
			} while(*cur_ptr);
			return kstd::string((char*) m_ptr);
		});
	};

	/**
	 * Performs a memcpy with this pointer as the source.
	 * @param dest The destination buffer.
	 * @param offset The offset (in sizeof(T)) to start reading at.
	 * @param count The number of T elements to copy to the destination buffer.
	 */
	void read(T* dest, size_t offset, size_t count) const {
		checked<void>(false, offset, count, [&]() {
			memcpy(dest, m_ptr + offset, sizeof(T) * count);
		});
	}

	/**
	 * Performs a memcpy with this pointer as the source.
	 * @param dest The safe destination buffer.
	 * @param offset The offset (in sizeof(T)) to start reading at.
	 * @param count The number of T elements to copy to the destination buffer.
	 */
	void read(SafePointer dest, size_t offset, size_t count) const {
		checked<void>(false, offset, count, [&]() {
			dest.write(m_ptr + offset, count);
		});
	}

	/**
	 * Performs a memcpy with this pointer as the source.
	 * @param dest The destination buffer.
	 * @param count The number of T elements to copy to the destination buffer.
	 */
	inline void read(T* dest, size_t count) const {
		read(dest, 0, count);
	}

	/**
	 * Performs a memcpy with this pointer as the source.
	 * @param dest The safe destination buffer.
	 * @param count The number of T elements to copy to the destination buffer.
	 */
	void read(SafePointer dest, size_t count) const {
		read(dest, 0, count);
	}

	/**
	 * Performs a memcpy with this pointer as the destination.
	 * @param source The source buffer.
	 * @param offset The offset (in sizeof(T)) to start writing at.
	 * @param count The number of T elements to copy from the destination buffer.
	 */
	void write(const T* source, size_t offset, size_t count) const {
		checked<void>(true, offset, count, [&]() {
			memcpy(m_ptr + offset, source, count * sizeof(T));
		});
	}

	/**
	 * Performs a memcpy with this pointer as the destination.
	 * @param source The source buffer.
	 * @param offset The offset (in sizeof(T)) to start writing at.
	 * @param count The number of T elements to copy from the destination buffer.
	 */
	void write(SafePointer source, size_t offset, size_t count) const {
		checked<void>(true, offset, count, [&]() {
			source.read(m_ptr + offset, count);
		});
	}

	/**
	 * Performs a memcpy with this pointer as the destination.
	 * @param source The source buffer.
	 * @param count The number of T elements to copy from the destination buffer.
	 */
	inline void write(const T* source, size_t count) const {
		write(source, 0, count);
	}

	/**
	 * Performs a memcpy with this pointer as the destination.
	 * @param source The source buffer.
	 * @param count The number of T elements to copy from the destination buffer.
	 */
	void write(SafePointer source, size_t count) const {
		write(source, 0, count);
	}

	/**
	 * Performs a memset with this pointer as the destination.
	 * @param value The value to set.
	 * @param offset The offset (in sizeof(T)) to start writing at.
	 * @param count The number of T elements to copy from the destination buffer.
	 */
	void memset(T value, size_t offset, size_t count) const {
		checked<void>(true, offset, count, [&]() {
			::memset(m_ptr + offset, value, sizeof(T) * count);
		});
	}

	/**
	 * @return Whether or not the pointer is null.
	 */
	explicit operator bool() const {
		return m_ptr;
	}

	/**
	 * Performs a lambda with the pointers at the given offset and count checked.
	 * @param permission The permission needed.
	 * @param offset The offset (in sizeof(T)) into the pointer to start checking.
	 * @param count The number of T elements to check.
	 */
	template<typename R, typename F>
	R checked(bool write, off_t offset, size_t count, F&& lambda) const {
		if(!m_is_user)
			return lambda();
		auto* process = TaskManager::current_process();
		return process->vm_space()->lock().synced<R>([&]() {
			auto page_start_ptr = ((size_t) (m_ptr + offset) / PAGE_SIZE) * PAGE_SIZE;
			auto page_end_ptr = (((size_t) (m_ptr + offset + count) - 1) / PAGE_SIZE) * PAGE_SIZE;
			for(size_t ptr = page_start_ptr; ptr <= page_end_ptr; ptr += PAGE_SIZE) {
				process->check_ptr((const void*) ptr, write);
			}
			return lambda();
		});
	}

	template<typename R>
	SafePointer<R> as() {
		return SafePointer<R>((R*) m_ptr, m_is_user);
	}

	SafePointer operator+(ssize_t offset) const {
		return SafePointer(m_ptr + offset, m_is_user);
	}

	SafePointer offset(ssize_t offset) const {
		return operator+(offset);
	}

private:
	T* m_ptr = nullptr;
	bool m_is_user = false;
};

template<typename T>
class KernelPointer: public SafePointer<T> {
public:
	KernelPointer(T* raw_ptr): SafePointer<T>(raw_ptr, false) {}
};

template<typename T>
class UserspacePointer: public SafePointer<T> {
public:
	UserspacePointer(T* raw_ptr): SafePointer<T>(raw_ptr, true) {}
};