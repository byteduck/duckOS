/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "AnonymousVMObject.h"
#include "MemoryManager.h"
#include "../kstd/cstring.h"

SpinLock AnonymousVMObject::s_shared_lock;
int AnonymousVMObject::s_cur_shm_id = 1;
kstd::map<int, kstd::Weak<AnonymousVMObject>> AnonymousVMObject::s_shared_objects;

AnonymousVMObject::~AnonymousVMObject() {
	if(m_is_shared) {
		LOCK(s_shared_lock);
		s_shared_objects.erase(m_shm_id);
	}
}

ResultRet<kstd::Arc<AnonymousVMObject>> AnonymousVMObject::alloc(size_t size) {
	size_t num_pages = kstd::ceil_div(size, PAGE_SIZE);
	auto pages = TRY(MemoryManager::inst().alloc_physical_pages(num_pages));
	auto object = kstd::Arc<AnonymousVMObject>(new AnonymousVMObject(pages));
	auto tmp_mapped = MM.map_object(object);
	memset((void*) tmp_mapped->start(), 0, object->size());
	return object;
}

ResultRet<kstd::Arc<AnonymousVMObject>> AnonymousVMObject::alloc_contiguous(size_t size) {
	size_t num_pages = kstd::ceil_div(size, PAGE_SIZE);
	auto pages = TRY(MemoryManager::inst().alloc_contiguous_physical_pages(num_pages));
	auto object = kstd::Arc<AnonymousVMObject>(new AnonymousVMObject(pages));
	auto tmp_mapped = MM.map_object(object);
	memset((void*) tmp_mapped->start(), 0, object->size());
	return object;
}

ResultRet<kstd::Arc<AnonymousVMObject>> AnonymousVMObject::map_to_physical(PhysicalAddress start, size_t size) {
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
	object->m_fork_action = ForkAction::Ignore;
	return kstd::Arc<AnonymousVMObject>(object);
}

ResultRet<kstd::Arc<AnonymousVMObject>> AnonymousVMObject::get_shared(int id) {
	LOCK(s_shared_lock);
	auto node = s_shared_objects.find_node(id);
	if(node) {
		auto ptr = node->data.second;
		ASSERT(ptr);
		return ptr.lock();
	}
	return Result(ENOENT);
}

void AnonymousVMObject::share(pid_t pid, VMProt prot) {
	LOCK(m_lock);
	if(!m_is_shared) {
		LOCK_N(s_shared_lock, shared_lock);
		m_shm_id = s_cur_shm_id++;
		s_shared_objects.insert({m_shm_id, self()});
		m_is_shared = true;
		m_fork_action = ForkAction::Ignore;
	}
	m_shared_permissions[pid] = prot;
}


ResultRet<VMProt> AnonymousVMObject::get_shared_permissions(pid_t pid) {
	LOCK(m_lock);
	auto node = m_shared_permissions.find_node(pid);
	if(!node)
		return Result(ENOENT);
	return node->data.second;
}

AnonymousVMObject::AnonymousVMObject(kstd::vector<PageIndex> physical_pages):
		VMObject(kstd::move(physical_pages)) {}
