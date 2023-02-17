/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "VMSpace.h"
#include "MemoryManager.h"
#include "AnonymousVMObject.h"
#include "../kstd/cstring.h"
#include "InodeVMObject.h"
#include "../kstd/KLog.h"

const VMProt VMSpace::default_prot = {
	.read = true,
	.write = true,
	.execute = true
};

VMSpace::VMSpace(VirtualAddress start, size_t size, PageDirectory& page_directory):
	m_start(start),
	m_size(size),
	m_page_directory(page_directory)
{}

VMSpace::~VMSpace() = default;

kstd::Arc<VMSpace> VMSpace::fork(PageDirectory& page_directory, kstd::vector<kstd::Arc<VMRegion>>& regions_vec) {
	LOCK(m_lock);
	auto new_space = kstd::Arc<VMSpace>(new VMSpace(m_start, m_size, page_directory));
	new_space->m_used = m_used;

	// Clone regions
	for(auto& region_pair : m_region_map) {
		auto region = region_pair.second;
		if(!region) {
			// Reserved space
			ASSERT(new_space->m_region_map.insert({region_pair.first, nullptr}));
		}

		kstd::Arc<VMObject> new_object = region->object();
		// Mark as CoW / share if necessary
		switch(region->object()->fork_action()) {
			case VMObject::ForkAction::BecomeCoW: {
				auto new_object_res = region->object()->clone();
				m_page_directory.map(*region);
				if(new_object_res.is_error()) {
					KLog::err("VMSpace", "Could not clone a VMObject: %d!", new_object_res.code());
					break;
				}
				new_object = new_object_res.value();
				[[fallthrough]];
			}
			case VMObject::ForkAction::Share: {
				auto new_vmRegion = kstd::Arc<VMRegion>::make(
						new_object,
						new_space,
						region->range(),
						region->object_start(),
						region->prot());
				ASSERT(new_space->m_region_map.insert({region_pair.first, new_vmRegion.get()}));
				page_directory.map(*new_vmRegion);
				regions_vec.push_back(new_vmRegion);
				break;
			}
			case VMObject::ForkAction::Ignore:
				break;
		}
	}

	return new_space;
}

ResultRet<kstd::Arc<VMRegion>> VMSpace::map_object(kstd::Arc<VMObject> object, VMProt prot, VirtualRange range, VirtualAddress object_start) {
	// Use the size of the range if defined, or the remainder of the object size if not.
	if(!range.size)
		range.size = object->size() - object_start;

	// Validate alignments and size
	if(range.start % PAGE_SIZE != 0 || range.size % PAGE_SIZE != 0 || object_start % PAGE_SIZE != 0 || object_start + range.size > object->size())
		return Result(EINVAL);

	LOCK(m_lock);

	// Find the appropriate range to use
	if(range.start) {
		// Make sure there's no region occupying that space already
		if(m_region_map.find_node(range))
			return Result(ENOSPC);
	} else {
		// Map in the first available space
		range.start = TRY(find_free_space(range.size));
	}

	// Create and map the region
	auto vmRegion = kstd::make_shared<VMRegion>(
			object,
			self(),
			range,
			object_start,
			prot);
	auto new_node = m_region_map.insert({range, vmRegion.get()});
	ASSERT(new_node);
	m_page_directory.map(*vmRegion);
	m_used += range.size;
	return vmRegion;
}

ResultRet<kstd::Arc<VMRegion>> VMSpace::map_stack(kstd::Arc<VMObject> object, VMProt prot) {
	LOCK(m_lock);

	// If there's room at the very end, put it there
	if(!m_region_map.find_node(end() - object->size()))
		return map_object(object, prot, {end() - object->size(), object->size()});

	// Try mapping right before every object starting from the end until we find space
	auto iter = m_region_map.end() - 1;
	while(iter != m_region_map.end()) {
		auto map_res = map_object(object, prot, {iter->first.start - object->size(), object->size()});
		if(!map_res.is_error())
			return map_res.value();
		iter--;
	}

	// No room :(
	return Result(ENOSPC);
}

Result VMSpace::unmap_region(VMRegion& region) {
	return unmap_region(region.start());
}

Result VMSpace::unmap_region(VirtualAddress address) {
	LOCK(m_lock);
	auto node = m_region_map.find_node(address);
	if(node) {
		auto region = node->data.second;
		if(region) {
			if(region->start() != address)
				return Result(ENOENT);
			region->m_space.reset();
			m_page_directory.unmap(*region);
		}
		m_region_map.erase(address);
		m_used -= node->data.first.size;
		return Result(SUCCESS);
	}
	return Result(ENOENT);
}

ResultRet<kstd::Arc<VMRegion>> VMSpace::get_region_at(VirtualAddress address) {
	LOCK(m_lock);
	auto node = m_region_map.find_node(address);
	if(node) {
		auto region = node->data.second;
		if(!region || node->data.first.start != address)
			return Result(ENOENT);
		return region->self();
	}
	return Result(ENOENT);
}

ResultRet<kstd::Arc<VMRegion>> VMSpace::get_region_containing(VirtualAddress address) {
	LOCK(m_lock);
	auto node = m_region_map.find_node(address);
	if(node) {
		auto region = node->data.second;
		if(!region)
			return Result(ENOENT);
		return region->self();
	}
	return Result(ENOENT);
}

Result VMSpace::reserve_region(VirtualAddress start, size_t size) {
	LOCK(m_lock);
	if(!m_region_map.insert({{start, size}, nullptr}))
		return Result(ENOSPC);
	m_used += size;
	return Result(SUCCESS);
}

Result VMSpace::try_pagefault(PageFault fault) {
	LOCK(m_lock);
	auto node = m_region_map.find_node(fault.address);
	if(!node)
		return Result(ENOENT);

	auto vmRegion = node->data.second;
	if(!vmRegion)
		return Result(ENOENT);

	// First, sanity check. If the region doesn't have the proper permissions, we can just fail here.
	auto prot = vmRegion->prot();
	if(
		(!prot.read && fault.type == PageFault::Type::Read) ||
		(!prot.write && fault.type == PageFault::Type::Write) ||
		(!prot.execute && fault.type == PageFault::Type::Execute)
	) {
		return Result(EINVAL);
	}

	PageIndex error_page = (fault.address - vmRegion->start()) / PAGE_SIZE;

	// Check if the region is a mapped inode.
	if(vmRegion->object()->is_inode()) {
		auto inode_object = kstd::static_pointer_cast<InodeVMObject>(vmRegion->object());

		// Check to see if it needs to be read in
		LOCK_N(inode_object->lock(), inode_locker);
		if(inode_object->physical_page_index(error_page)) {
			// This page may be marked CoW, so copy it if it is
			if(vmRegion->prot().write && inode_object->page_is_cow(error_page)) {
				auto res = vmRegion->m_object->try_cow_page(error_page);
				if(res.is_error())
					return res;
			}

			// Or, we may have encountered a race where the page was created by another thread after the fault.
			m_page_directory.map(*vmRegion, VirtualRange { error_page * PAGE_SIZE, PAGE_SIZE });
			return Result(SUCCESS);
		}

		// Allocate a new physical page.
		auto new_page = TRY(MM.alloc_physical_page());

		// We read directly from the shared VMObject if this page exists in it.
		auto inode = inode_object->inode();
		auto shared_object = inode->shared_vm_object();
		PageIndex shared_page_index = error_page + (vmRegion->object_start() / PAGE_SIZE);
		auto shared_page = shared_object->physical_page_index(shared_page_index);
		if(shared_object != inode_object && shared_page) {
			MM.copy_page(shared_page, new_page);
		} else {
			// Read the appropriate part of the file into the buffer.
			kstd::Arc<uint8_t> buf((uint8_t*) kmalloc(PAGE_SIZE));
			ssize_t nread = inode->read(error_page * PAGE_SIZE + vmRegion->object_start(), PAGE_SIZE, KernelPointer<uint8_t>(buf.get()), nullptr);
			if(nread < 0)
				return Result(-nread);

			// Read the contents of the buffer into the newly allocated physical page.
			MM.with_quickmapped(new_page, [&](void* page_buf) {
				memcpy_uint32((uint32_t*) page_buf, (uint32_t*) buf.get(), PAGE_SIZE / sizeof(uint32_t));
			});
		}

		// Remap the page.
		inode_object->physical_page_index(error_page) = new_page;
		m_page_directory.map(*vmRegion, VirtualRange { error_page * PAGE_SIZE, PAGE_SIZE });

		return Result(SUCCESS);
	}

	// CoW if the region is writeable.
	if(vmRegion->prot().write) {
		auto result = vmRegion->m_object->try_cow_page(error_page);
		if(result.is_success())
			m_page_directory.map(*vmRegion, VirtualRange { error_page * PAGE_SIZE, PAGE_SIZE });
		return result;
	}

	return Result(EINVAL);
}

ResultRet<VirtualAddress> VMSpace::find_free_space(size_t size) {
	LOCK(m_lock);
	VirtualRange range = { m_start, size };
	auto iter = m_region_map.begin();
	while(iter != m_region_map.end()) {
		ASSERT(iter->first.start >= range.start);
		if(iter->first.start - range.start >= size)
			return range.start;
		range.start = iter->first.end();
		iter++;
	}
	if(range.end() > end())
		return Result(ENOSPC);
	return range.start;
}

size_t VMSpace::calculate_regular_anonymous_total() {
	LOCK(m_lock);
	size_t total = 0;
	for(auto& region_pair : m_region_map) {
		auto region = region_pair.second;
		if(!region)
			continue;
		if(region->object()->is_anonymous()) {
			auto anon_object = kstd::static_pointer_cast<AnonymousVMObject>(region->object());
			if(!anon_object->is_shared())
				total += anon_object->size();
		}
	}
	return total;
}
