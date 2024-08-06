/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "AnonymousVMObject.h"
#include "MemoryManager.h"

Mutex AnonymousVMObject::s_shared_lock {"AnonymousVMObject::Shared"};
int AnonymousVMObject::s_cur_shm_id = 1;
kstd::map<int, kstd::Weak<AnonymousVMObject>> AnonymousVMObject::s_shared_objects;

AnonymousVMObject::~AnonymousVMObject() {
	if(m_is_shared) {
		LOCK(s_shared_lock);
		s_shared_objects.erase(m_shm_id);
	}
}

ResultRet<kstd::Arc<AnonymousVMObject>> AnonymousVMObject::alloc(size_t size, kstd::string name, bool commit) {
	size_t num_pages = kstd::ceil_div(size, PAGE_SIZE);

	// If we requested uncommitted pages, don't allocate any physical pages yet.
	if (!commit)
		return kstd::Arc<AnonymousVMObject>(new AnonymousVMObject(name, num_pages, false));

	// If we requested committed pages, alloc them now.
	auto pages = TRY(MemoryManager::inst().alloc_physical_pages(num_pages));
	auto object = kstd::Arc<AnonymousVMObject>(new AnonymousVMObject(name, pages, false));
	auto tmp_mapped = MM.map_object(object);
	memset((void*) tmp_mapped->start(), 0, object->size());
	return object;
}

ResultRet<kstd::Arc<AnonymousVMObject>> AnonymousVMObject::alloc_contiguous(size_t size, kstd::string name) {
	size_t num_pages = kstd::ceil_div(size, PAGE_SIZE);
	auto pages = TRY(MemoryManager::inst().alloc_contiguous_physical_pages(num_pages));
	auto object = kstd::Arc<AnonymousVMObject>(new AnonymousVMObject(name, pages, false));
	auto tmp_mapped = MM.map_object(object);
	memset((void*) tmp_mapped->start(), 0, object->size());
	return object;
}

ResultRet<kstd::Arc<AnonymousVMObject>> AnonymousVMObject::map_to_physical(PhysicalAddress start, size_t size, Type type) {
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

	auto object = new AnonymousVMObject("Physical Mapping", kstd::move(pages), false);
	object->m_num_committed_pages = 0; // Hack - but we don't want this counting towards our memory total.
	object->m_fork_action = ForkAction::Share;
	object->m_type = type;
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
	LOCK(m_page_lock);
	if(!m_is_shared) {
		LOCK_N(s_shared_lock, shared_lock);
		m_shared_owner = pid;
		m_shm_id = s_cur_shm_id++;
		s_shared_objects.insert({m_shm_id, self()});
		m_is_shared = true;
		m_fork_action = ForkAction::Ignore;
	}
	m_shared_permissions[pid] = prot;
}


ResultRet<VMProt> AnonymousVMObject::get_shared_permissions(pid_t pid) {
	LOCK(m_page_lock);
	auto node = m_shared_permissions.find_node(pid);
	if(!node)
		return Result(ENOENT);
	return node->data.second;
}

ResultRet<kstd::Arc<VMObject>> AnonymousVMObject::clone() {
	LOCK(m_page_lock);
	ASSERT(!is_shared());
	become_cow_and_ref_pages();
	auto new_object = kstd::Arc(new AnonymousVMObject(m_name, m_physical_pages, true));
	return kstd::static_pointer_cast<VMObject>(new_object);
}

ResultRet<bool> AnonymousVMObject::try_fault_in_page(PageIndex page) {
	LOCK(m_page_lock);
	if (page > m_physical_pages.size())
		return Result(EINVAL);
	if (m_physical_pages[page])
		return false;
	m_physical_pages[page] = TRY(MM.alloc_physical_page());
	m_num_committed_pages++;
	MM.with_quickmapped(m_physical_pages[page], [](void* pagemem) {
		memset(pagemem, 0, PAGE_SIZE);
	});
	return true;
}

size_t AnonymousVMObject::num_committed_pages() const {
	return m_num_committed_pages;
}

AnonymousVMObject::AnonymousVMObject(kstd::string name, kstd::vector<PageIndex> physical_pages, bool cow):
		VMObject(kstd::move(name), kstd::move(physical_pages), cow), m_num_committed_pages(m_physical_pages.size()) {}

AnonymousVMObject::AnonymousVMObject(kstd::string name, size_t n_pages, bool cow):
	VMObject(kstd::move(name), kstd::vector(n_pages, (PageIndex) 0), cow) {}
