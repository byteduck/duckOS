/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "../Atomic.h"
#include "../kstd/kstddef.h"
#include "Memory.h"
#include "../kstd/kstdio.h"

union PhysicalPage {
public:
	void ref() {
		if(allocated.ref_count.add(1) == 0xFFFF)
			PANIC("PPAGE_REF_COUNT_OVERFLOW", "A physical page was referenced too many times and overflowed.");
	}

	void unref() {
		auto prev_value = allocated.ref_count.sub(1);
		ASSERT(prev_value);
		if(prev_value == 1)
			release();
	}

	PageIndex index() const;

	inline void* ptr() const {
		return (void*) (index() * PAGE_SIZE);
	}

	inline PhysicalAddress paddr() const {
		return index() * PAGE_SIZE;
	}

	/// When a page is in use, this struct is used to keep count of its references.
	struct {
		Atomic<uint16_t, MemoryOrder::AcqRel> ref_count;
		bool reserved; // If true, deref() will not free anything
	} allocated;

	/// When a page is free, this struct is used to keep track of the next free page for the buddy block system
	struct {
		int16_t next; //< The index of the next free page in this page's bucket.
		int16_t prev; //< The index of the previous free page in this page's bucket.
	} free;

private:

	void release();

};
