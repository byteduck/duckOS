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

private:
	explicit AnonymousVMObject(kstd::vector<PageIndex> physical_pages);

};
