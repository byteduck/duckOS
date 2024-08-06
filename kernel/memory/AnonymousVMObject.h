/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "VMObject.h"
#include "../kstd/Arc.h"
#include "../Result.hpp"
#include "../kstd/map.hpp"
#include "../kstd/unix_types.h"
#include "../tasking/Mutex.h"
#include "VMRegion.h"

class AnonymousVMObject: public VMObject {
public:
	~AnonymousVMObject() override;

	enum class Type {
		Normal, Device
	};

	/**
	 * Allocates a new anonymous VMObject.
	 * @param size The minimum size, in bytes, of the object.
	 * @param name The name of the object.
	 * @param commit Whether to immediately commit physical pages.
	 * @return The newly allocated object, if successful.
	 */
	static ResultRet<kstd::Arc<AnonymousVMObject>> alloc(size_t size, kstd::string name = "anonymous", bool commit = false);

	/**
	 * Allocates a new anonymous VMObject backed by contiguous physical pages.
	 * @param size The minimum size, in bytes, of the object.
	 * @return The newly allocated object, if successful.
	 */
	static ResultRet<kstd::Arc<AnonymousVMObject>> alloc_contiguous(size_t size, kstd::string name = "anonymous");

	/**
	 * Creates an anonymous VMObject backed by existing physical pages.
	 * @param start The start address to map to. Will be rounded down to a page boundary.
	 * @param size The size to map. Will be rounded up to a page boundary.
	 * @param type Whether the mapping is mapped to normal memory or a device.
	 * @return The newly allocated object, if successful.
	 */
	static ResultRet<kstd::Arc<AnonymousVMObject>> map_to_physical(PhysicalAddress start, size_t size, Type type);

	/**
	 * Finds the existing anonymous VMObject with the given shared memory ID, if any.
	 * @param id The ID of the VMObject to find.
	 * @return The object if found, or an error if not.
	 */
	static ResultRet<kstd::Arc<AnonymousVMObject>> get_shared(int id);

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

	/**
	 * Sets the fork action of this object. Only use if you know what you're doing.
	 * @param action The action to take when forking a VMSpace with this object.
	 */
	void set_fork_action(ForkAction action) { m_fork_action = action; }

	bool is_shared() const { return m_is_shared; }
	pid_t shared_owner() const { return m_shared_owner; }
	int shm_id() const { return m_shm_id; }

	// VMObject
	bool is_anonymous() const override { return true; }
	bool is_device() const override { return m_type == Type::Device; }
	ForkAction fork_action() const override { return m_fork_action; }
	ResultRet<kstd::Arc<VMObject>> clone() override;
	ResultRet<bool> try_fault_in_page(PageIndex page) override;
	size_t num_committed_pages() const override;


private:
	friend class MemoryManager;

	explicit AnonymousVMObject(kstd::string name, kstd::vector<PageIndex> physical_pages, bool cow);
	explicit AnonymousVMObject(kstd::string name, size_t n_pages, bool cow);

	static Mutex s_shared_lock;
	static int s_cur_shm_id;
	static kstd::map<int, kstd::Weak<AnonymousVMObject>> s_shared_objects;

	kstd::map<pid_t, VMProt> m_shared_permissions;
	bool m_is_shared = false;
	ForkAction m_fork_action = ForkAction::BecomeCoW;
	pid_t m_shared_owner;
	int m_shm_id = 0;
	size_t m_num_committed_pages = 0;
	Type m_type = Type::Normal;
};
