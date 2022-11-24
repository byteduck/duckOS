/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "VMObject.h"
#include "../kstd/shared_ptr.hpp"
#include "../Result.hpp"

class AnonymousVMObject: public VMObject {
public:
	/**
	 * Allocates a new anonymous VMObject.
	 * @param size The minimum size, in bytes, of the object.
	 * @return The newly allocated object, if successful.
	 */
	static ResultRet<Ptr<AnonymousVMObject>> alloc(size_t size);

	/**
	 * Allocates a new anonymous VMObject backed by contiguous physical pages.
	 * @param size The minimum size, in bytes, of the object.
	 * @return The newly allocated object, if successful.
	 */
	static ResultRet<Ptr<AnonymousVMObject>> alloc_contiguous(size_t size);

	/**
	 * Creates an anonymous VMObject backed by existing physical pages.
	 * @param start The start address to map to. Will be rounded down to a page boundary.
	 * @param size The size to map. Will be rounded up to a page boundary.
	 * @return The newly allocated object, if successful.
	 */
	static ResultRet<Ptr<AnonymousVMObject>> map_to_physical(PhysicalAddress start, size_t size);

	// VMObject
	bool is_anonymous() const override { return true; }

private:
	explicit AnonymousVMObject(kstd::vector<PageIndex> physical_pages);

};
