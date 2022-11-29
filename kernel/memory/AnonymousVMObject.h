/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "VMObject.h"
#include "../kstd/shared_ptr.hpp"
#include "../Result.hpp"
#include "../kstd/map.hpp"
#include "../kstd/unix_types.h"
#include "../tasking/SpinLock.h"
#include "VMRegion.h"

class AnonymousVMObject: public VMObject {
public:
	enum class ForkAction {
		BecomeCoW, Ignore
	};

	~AnonymousVMObject();

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

	/**
	 * Finds the existing anonymous VMObject with the given shared memory ID, if any.
	 * @param id The ID of the VMObject to find.
	 * @return The object if found, or an error if not.
	 */
	static ResultRet<Ptr<AnonymousVMObject>> get_shared(int id);

	/**
	 * Shares the anonymous VMObject with the given process with the given permissions.
	 * @param pid The PID of the process to share with.
	 * @param prot The rights that the process should have.
	 */
	void share(pid_t pid, VMProt prot);

	/**
	 * Gets the shared memory permissions for this object for a given process.
	 * @param pid The pid to check permissions for.
	 * @return The permissions if set, or an error if not set.
	 */
	ResultRet<VMProt> get_shared_permissions(pid_t pid);

	bool is_shared() const { return m_is_shared; }
	int shm_id() const { return m_shm_id; }
	ForkAction fork_action() const { return m_fork_action; }

	// VMObject
	bool is_anonymous() const override { return true; }

private:
	friend class MemoryManager;

	explicit AnonymousVMObject(kstd::vector<PageIndex> physical_pages);

	static SpinLock s_shared_lock;
	static int s_cur_shm_id;
	static kstd::map<int, Weak<AnonymousVMObject>> s_shared_objects;

	SpinLock m_lock;
	kstd::map<pid_t, VMProt> m_shared_permissions;
	bool m_is_shared = false;
	int m_shm_id = 0;
	ForkAction m_fork_action = ForkAction::BecomeCoW;
};
