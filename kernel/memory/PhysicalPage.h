/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "../Atomic.h"
#include "../kstd/kstddef.h"
#include "Memory.h"

union PhysicalPage {
public:
	void ref() {
		allocated.ref_count.add(1);
	}

	void unref() {
		if(allocated.ref_count.sub(1) == 1)
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
		Atomic<uint32_t, MemoryOrder::AcqRel> ref_count;
	} allocated;

	/// When a page is free, this struct is used to keep track of the next free page for the buddy block system
	struct {
		int16_t next; //< The index of the next free page in this page's bucket.
		int16_t prev; //< The index of the previous free page in this page's bucket.
	} free;

private:

	void release();

};
