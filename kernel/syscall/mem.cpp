/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "../tasking/Process.h"
#include "../memory/SafePointer.h"
#include "../memory/AnonymousVMObject.h"
#include "../kstd/KLog.h"
#include "../api/mmap.h"
#include "../filesystem/FileDescriptor.h"
#include "../filesystem/InodeFile.h"
#include "../memory/InodeVMObject.h"

int Process::sys_shmcreate(UserspacePointer<shmcreate_args> args_p) {
	auto args = args_p.get();

	kstd::string name = "shared";
	if (args.name)
		name = UserspacePointer<const char>(args.name).str();
	auto object_res = AnonymousVMObject::alloc(args.size, name);
	if(object_res.is_error())
		return object_res.code();
	auto object = object_res.value();

	object->share(_pid, VMProt::RW);
	auto region_res = args.addr ? map_object(object, (VirtualAddress) args.addr, VMProt::RW) : map_object(object, VMProt::RW);
	if(region_res.is_error())
		return region_res.code();
	auto region = region_res.value();

	m_mem_lock.synced<void>([&]() {
		m_used_shmem += region->size();
	});

	shm ret;
	ret.size = region->size();
	ret.ptr = (void*) region->start();
	ret.id = object->shm_id();
	UserspacePointer<struct shm>(args.shm).set(ret);

	return SUCCESS;
}

int Process::sys_shmattach(int id, void* addr, UserspacePointer<struct shm> s) {
	auto do_shmattach = [&] () -> Result {
		// Find the object in question
		auto object = TRY(AnonymousVMObject::get_shared(id));

		// Check permissions
		auto perms = TRY(object->get_shared_permissions(_pid));
		if(!perms.read)
			return Result(ENOENT);

		// Map into our space
		auto region = TRY(addr ? _vm_space->map_object(object, perms, VirtualRange { (VirtualAddress) addr, object->size() }) : _vm_space->map_object(object, perms));
		LOCK(m_mem_lock);
		_vm_regions.push_back(region);

		m_used_shmem += region->size();

		// Setup the shm struct
		struct shm ret = {
				.ptr = (void*) region->start(),
				.size = region->size(),
				.id = object->shm_id()
		};
		s.set(ret);

		return Result(SUCCESS);
	};

	return do_shmattach().code();
}

int Process::sys_shmdetach(int id) {
	// Find the object in question
	auto object_res = AnonymousVMObject::get_shared(id);
	if(object_res.is_error())
		return object_res.code();
	auto object = object_res.value();

	// Remove it from our vm regions
	LOCK(m_mem_lock);
	for(size_t i = 0; i < _vm_regions.size(); i++) {
		if(_vm_regions[i]->object() == object) {
			m_used_shmem -= object->size();
			_vm_regions.erase(i);
			return SUCCESS;
		}
	}

	return ENOENT;
}

int Process::sys_shmallow(int id, pid_t pid, int perms) {
	// TODO: Sharing allowed regions that we didn't directly create
	if(perms & SHM_SHARE)
		return -EINVAL;

	if(!(perms &  (SHM_READ | SHM_WRITE)))
		return -EINVAL;
	if((perms & SHM_WRITE) && !(perms & SHM_READ))
		return -EINVAL;
	if(TaskManager::process_for_pid(pid).is_error())
		return -EINVAL;

	// Find the object in question
	auto object_res = AnonymousVMObject::get_shared(id);
	if(object_res.is_error())
		return object_res.code();

	// Set the perms
	object_res.value()->share(pid, VMProt {
			.read = (bool) (perms & SHM_READ),
			.write = (bool) (perms & SHM_WRITE),
			.execute = false
	});

	return SUCCESS;
}

Result Process::sys_mmap(UserspacePointer<struct mmap_args> args_ptr) {
	mmap_args args = args_ptr.get();
	LOCK(m_mem_lock);

	args.length = kstd::ceil_div(args.length, PAGE_SIZE) * PAGE_SIZE;

	kstd::Arc<VMObject> vm_object;
	kstd::Arc<VMRegion> region;
	VMProt prot = {
		.read = (bool) (args.prot & PROT_READ),
		.write = (bool) (args.prot & PROT_WRITE),
		.execute = (bool) (args.prot & PROT_EXEC)
	};

	// First, create an appropriate object
	if(args.flags & MAP_ANONYMOUS) {
		kstd::string name = "anonymous";
		if (args.name)
			name = UserspacePointer<const char>(args.name).str();
		vm_object = TRY(AnonymousVMObject::alloc(args.length, name));
	} else {
		if(args.fd >= _file_descriptors.size() || !_file_descriptors[args.fd])
			return Result(EBADF);
		auto file_desc = _file_descriptors[args.fd];
		if((!file_desc->readable() && prot.read) || (!file_desc->writable() && prot.write && (args.flags & MAP_SHARED)))
			return Result(EPERM);
		auto file = file_desc->file();
		if(!file || !file->is_inode())
			return Result(EBADF);
		auto inode = kstd::static_pointer_cast<InodeFile>(file)->inode();
		if(args.flags & MAP_SHARED)
			vm_object = inode->shared_vm_object(file_desc->path());
		else
			vm_object = InodeVMObject::make_for_inode(file_desc->path(), inode, InodeVMObject::Type::Private);
	}

	if(!vm_object)
		return Result(EINVAL);

	// Then, map it appropriately
	if(args.addr && (args.flags & MAP_FIXED)) {
		region = TRY(_vm_space->map_object(vm_object, prot, VirtualRange { (VirtualAddress) args.addr, args.length }, args.offset));
	} else {
		if(args.addr)
			KLog::warn("mmap", "mmap requested address without MAP_FIXED!");
		region = TRY(_vm_space->map_object(vm_object, prot, VirtualRange { 0, args.length }, args.offset));
	}

	if(!region)
		return Result(EINVAL);

	_vm_regions.push_back(region);
	UserspacePointer<void*>(args.addr_p).set((void*) region->start());
	return Result(SUCCESS);
}

int Process::sys_munmap(void* addr, size_t length) {
	// TODO: Unmap partial regions
	LOCK(m_mem_lock);
	// Find the region
	for(size_t i = 0; i < _vm_regions.size(); i++) {
		/* TODO: Size mismatch? */
		if(_vm_regions[i]->start() == (VirtualAddress) addr /* && _vm_regions[i]->size() == length*/) {
			_vm_regions.erase(i);
			return SUCCESS;
		}
	}
	KLog::warn("Process", "munmap() for {}({}) failed.", _name, _pid);
	return ENOENT;
}

int Process::sys_mprotect(void* addr, size_t length, int prot_flags) {
	// TODO: Protect partial regions
	LOCK(m_mem_lock);

	VMProt prot = {
			.read = (bool) (prot_flags & PROT_READ),
			.write = (bool) (prot_flags & PROT_WRITE),
			.execute = (bool) (prot_flags & PROT_EXEC)
	};

	// Find the region
	for(size_t i = 0; i < _vm_regions.size(); i++) {
		if(_vm_regions[i]->start() == (VirtualAddress) addr) {
			_vm_regions[i]->set_prot(prot);
			_page_directory->map(*_vm_regions[i]);
			return SUCCESS;
		}
	}

	KLog::warn("Process", "mprotect() for {}({}) failed.", _name, _pid);
	return ENOENT;
}