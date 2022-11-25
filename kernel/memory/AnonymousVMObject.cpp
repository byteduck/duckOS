/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "AnonymousVMObject.h"
#include "MemoryManager.h"
#include "../kstd/cstring.h"

ResultRet<Ptr<AnonymousVMObject>> AnonymousVMObject::alloc(size_t size) {
	size_t num_pages = kstd::ceil_div(size, PAGE_SIZE);
	auto pages = TRY(MemoryManager::inst().alloc_physical_pages(num_pages));
	auto object = Ptr<AnonymousVMObject>(new AnonymousVMObject(kstd::move(pages)));
	auto tmp_mapped = MM.map_object(object);
	memset((void*) tmp_mapped->start(), 0, object->size());
	return object;
}

ResultRet<Ptr<AnonymousVMObject>> AnonymousVMObject::alloc_contiguous(size_t size) {
	size_t num_pages = kstd::ceil_div(size, PAGE_SIZE);
	auto pages = TRY(MemoryManager::inst().alloc_contiguous_physical_pages(num_pages));
	auto object = Ptr<AnonymousVMObject>(new AnonymousVMObject(kstd::move(pages)));
	auto tmp_mapped = MM.map_object(object);
	memset((void*) tmp_mapped->start(), 0, object->size());
	return object;
}

ResultRet<Ptr<AnonymousVMObject>> AnonymousVMObject::map_to_physical(PhysicalAddress start, size_t size) {
	ASSERT((start / PAGE_SIZE) * PAGE_SIZE == start);

	PageIndex start_page = start / PAGE_SIZE;
	PageIndex num_pages = kstd::ceil_div(start + size, PAGE_SIZE) - start_page;

	// Detect overflow
	if(((start_page + num_pages) * PAGE_SIZE) < (start_page * PAGE_SIZE))
		return Result(EINVAL);

	kstd::vector<PageIndex> pages;
	pages.reserve(num_pages);
	for(size_t i = 0; i < num_pages; i++) {
		new(&MM.get_physical_page(start_page + i)) PhysicalPage{.allocated {.ref_count = 1, .reserved = true}};
		pages.push_back(start_page + i);
	}

	auto object = new AnonymousVMObject(kstd::move(pages));
	return Ptr<AnonymousVMObject>(object);
}

AnonymousVMObject::AnonymousVMObject(kstd::vector<PageIndex> physical_pages):
		VMObject(kstd::move(physical_pages)) {}
