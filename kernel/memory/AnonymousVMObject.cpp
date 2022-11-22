/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "AnonymousVMObject.h"
#include "MemoryManager.h"

ResultRet<Ptr<AnonymousVMObject>> AnonymousVMObject::alloc(size_t size) {
	size_t num_pages = kstd::ceil_div(size, PAGE_SIZE);
	auto pages = TRY(MemoryManager::inst().alloc_physical_pages(num_pages));
	auto object = new AnonymousVMObject(pages);
	return kstd::shared_ptr<AnonymousVMObject>(object);
}

ResultRet<Ptr<AnonymousVMObject>> AnonymousVMObject::alloc_contiguous(size_t size) {
	size_t num_pages = kstd::ceil_div(size, PAGE_SIZE);
	auto pages = TRY(MemoryManager::inst().alloc_contiguous_physical_pages(num_pages));
	auto object = new AnonymousVMObject(pages);
	return kstd::shared_ptr<AnonymousVMObject>(object);
}

AnonymousVMObject::AnonymousVMObject(kstd::vector<PageIndex> physical_pages):
		VMObject(kstd::move(physical_pages)) {}
