/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include <kernel/tasking/TaskManager.h>
#include "MemoryManager.h"

template<typename T>
class SafePointer {
	virtual T* raw() const = 0;
	virtual T get(int index = 0) const = 0;
	virtual void set(int index, const T& value) const = 0;
	virtual void set(const T& value) const = 0;
	virtual kstd::string str() const = 0;
	virtual void* memcpy(void* dest, size_t len) const = 0;
	virtual explicit operator bool() const = 0;
};

template<typename T>
class UserspacePointer: SafePointer<T> {
public:
	UserspacePointer(T* raw_ptr): m_ptr(raw_ptr) {}

	T* raw() const { return m_ptr; }

	/**
	 * Copies the value pointed to by this pointer from user memory.
	 * @param index An optional index into this pointer as an array.
	 * @return The value copied from userspace at the specified index.
	 */
	T get(int index = 0) const {
		auto* proc = TaskManager::current_thread()->process();
		return proc->page_directory()->lock().synced<T>([=]() {
			check_ptr(proc, index);
			return m_ptr[index];
		});
	}

	/**
	 * Sets the value pointed to by this pointer in user memory.
	 * @param index An index into this pointer as an array.
	 */
	void set(int index, const T& value) const {
		auto* proc = TaskManager::current_thread()->process();
		proc->page_directory()->lock().synced([=]() {
			check_ptr(proc, index);
			m_ptr[index] = value;
		});
	}

	/**
	 * Sets the value pointed to by this pointer to the given value.
	 * @param value The value to set.
	 */
	inline void set(const T& value) const {
		set(0, value);
	}

	/**
	 * Casts this pointer to a char pointer, and then safely makes a string from it.
	 * @return A string made from the C string that this pointer points to.
	 */
	kstd::string str() const {
		auto* proc = TaskManager::current_thread()->process();
		return proc->page_directory()->lock().synced<kstd::string>([=]() {
			auto cur_ptr = (char*) m_ptr;
			proc->check_ptr(cur_ptr);
			auto last_checked_page = (size_t) cur_ptr / PAGE_SIZE;
			do {
				if((size_t) cur_ptr / PAGE_SIZE != last_checked_page) {
					proc->check_ptr(cur_ptr);
					last_checked_page = (size_t) cur_ptr / PAGE_SIZE;
				}
				cur_ptr++;
			} while(*cur_ptr);
			return kstd::string((char*) m_ptr);
		});
	};

	/**
	 * Performs a memcpy with this pointer as the source.
	 */
	inline void* memcpy(void* dest, size_t len) const {
		auto* proc = TaskManager::current_thread()->process();
		return proc->page_directory()->lock().synced<void*>([=]() {
			auto page_start_ptr = ((size_t) m_ptr / PAGE_SIZE) * PAGE_SIZE;
			auto page_end_ptr = (((size_t) m_ptr + len - 1) / PAGE_SIZE) * PAGE_SIZE;
			for(size_t ptr = page_start_ptr; ptr <= page_end_ptr; ptr += PAGE_SIZE) {
				proc->check_ptr((const void*) ptr);
			}
			return ::memcpy(dest, (const void*) m_ptr, len);
		});
	}

	/**
	 * @return Whether or not the pointer is null.
	 */
	explicit operator bool() const {
		return m_ptr;
	}

private:
	/**
	 * Checks for the validity of the pointer to the element at index i.
	 */
	void check_ptr(Process* process, size_t i) const {
		auto page_start_ptr = (((size_t) m_ptr + sizeof(T) * i) / PAGE_SIZE) * PAGE_SIZE;
		auto page_end_ptr = (((size_t) m_ptr + sizeof(T) * (i + 1) - 1) / PAGE_SIZE) * PAGE_SIZE;
		for(size_t ptr = page_start_ptr; ptr <= page_end_ptr; ptr += PAGE_SIZE) {
			process->check_ptr((const void*) ptr);
		}
	}

	T* const m_ptr;
};

template<typename T>
class KernelPointer {
	KernelPointer(T* raw_ptr): m_ptr(raw_ptr) {}

	inline T* raw() const { return m_ptr; }
	inline T get(int index = 0) { return m_ptr[index]; }
	inline void set(int index, const T& value) const { m_ptr[index] = value; }
	inline void set(const T& value) const { *m_ptr = value; }
	inline kstd::string str() const { return kstd::string((char*) m_ptr); }
	inline void* memcpy(void* dest, size_t len) const { return ::memcpy(m_ptr, dest, len); }
	inline explicit operator bool() const { return m_ptr; }

private:
	T* const m_ptr;
};