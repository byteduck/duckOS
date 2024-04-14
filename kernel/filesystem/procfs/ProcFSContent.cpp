/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "ProcFSContent.h"
#include <kernel/memory/InodeVMObject.h>
#include <kernel/memory/AnonymousVMObject.h>
#include <kernel/StackWalker.h>
#include <kernel/KernelMapper.h>
#include <kernel/time/TimeManager.h>
#include <kernel/device/DiskDevice.h>

ResultRet<kstd::string> ProcFSContent::mem_info() {
	char numbuf[12];
	kstd::string str;

	str += "[mem]\nusable = ";
	itoa((int) MM.usable_mem(), numbuf, 10);
	str += numbuf;

	str += "\nused = ";
	itoa((int) MM.used_pmem(), numbuf, 10);
	str += numbuf;

	str += "\nreserved = ";
	itoa((int) MM.reserved_pmem(), numbuf, 10);
	str += numbuf;

	str += "\nkvirt = ";
	itoa((int) MM.kernel_vmem(), numbuf, 10);
	str += numbuf;

	str += "\nkphys = ";
	itoa((int) MM.kernel_pmem(), numbuf, 10);
	str += numbuf;

	str += "\nkheap = ";
	itoa((int) MM.kernel_heap(), numbuf, 10);
	str += numbuf;

	str += "\nkcache = ";
	itoa((int) DiskDevice::used_cache_memory(), numbuf, 10);
	str += numbuf;
	str += "\n";

	return str;
}

ResultRet<kstd::string> ProcFSContent::uptime() {
	char numbuf[12];
	itoa(TimeManager::uptime().tv_sec, numbuf, 10);
	kstd::string str = numbuf;
	str += "\n";
	return str;
}

ResultRet<kstd::string> ProcFSContent::cpu_info() {
	char numbuf[4];
	double percent_used = (1.00 - TimeManager::percent_idle()) * 100.0;

	kstd::string str = "[cpu]\nutil = ";

	itoa((int) percent_used, numbuf, 10);
	str += numbuf;
	str += ".";

	percent_used -= (int) percent_used;
	if(percent_used == 0)
		str += "0";
	int num_decimals = 0;
	while(percent_used > 0 && num_decimals < 3) {
		percent_used *= 10;
		itoa((int)percent_used, numbuf, 10);
		str += numbuf;
		percent_used -= (int) percent_used;
		num_decimals++;
	}
	return str;
}

ResultRet<kstd::string> ProcFSContent::status(pid_t pid) {
	const char* PROC_STATE_NAMES[] = {"Running", "Zombie", "Dead", "Sleeping", "Stopped"};

	auto proc = TRY(TaskManager::process_for_pid(pid));

	char numbuf[12];
	kstd::string str;

	str += "[proc]\nname = ";
	str += proc->name();

	str += "\nstate = ";
	itoa(proc->all_threads_state(), numbuf, 10);
	str += numbuf;

	str += "\nstate_name = ";
	str += PROC_STATE_NAMES[proc->all_threads_state()];

	str += "\npid = ";
	itoa(proc->pid(), numbuf, 10);
	str += numbuf;

	str += "\nppid = ";
	itoa(proc->ppid(), numbuf, 10);
	str += numbuf;

	str += "\nuid = ";
	itoa(proc->user().euid, numbuf, 10);
	str += numbuf;

	str += "\ngid = ";
	itoa(proc->user().egid, numbuf, 10);
	str += numbuf;

	str += "\npmem = ";
	itoa(proc->used_pmem(), numbuf, 10);
	str += numbuf;

	str += "\nvmem = ";
	itoa(proc->used_vmem(), numbuf, 10);
	str += numbuf;

	str += "\nshmem = ";
	itoa(proc->used_shmem(), numbuf, 10);
	str += numbuf;

	str += "\nthreads = ";
	auto& threads = proc->threads();
	for (size_t i = 0; i < threads.size(); i++) {
		itoa(threads[i], numbuf, 10);
		str += numbuf;
		if (i != threads.size() - 1)
			str += ",";
	}

	str += "\n";

	return str;
}

ResultRet<kstd::string> ProcFSContent::stacks(pid_t pid) {
	auto proc = TRY(TaskManager::process_for_pid(pid));
	kstd::string str;
	char numbuf[12];
	for (auto& tid : proc->threads()) {
		auto thread = proc->get_thread(tid);
		if (!thread)
			continue;
		itoa(tid, numbuf, 10);
		str += "<";
		str += numbuf;
		str += ">\n";
		StackWalker::Frame* stk_frame = nullptr;
		static constexpr int num_frames = 32;
		uintptr_t stk_buf[num_frames];
		do {
			stk_frame = StackWalker::walk_stack(thread, stk_buf, num_frames, stk_frame);
			int i = 0;
			while(stk_buf[i] && i < num_frames) {
				itoa(stk_buf[i], numbuf, 16);
				str += "0x";
				str += numbuf;
				if (stk_buf[i] >= HIGHER_HALF) {
					auto symbol = KernelMapper::get_symbol(stk_buf[i]);
					str += "\t";
					str += symbol->name;
					str += " + ";
					itoa(stk_buf[i] - symbol->location, numbuf, 10);
					str += numbuf;
				}
				str += "\n";
				i++;
			}
		} while(stk_frame);
	}
	return str;
}

ResultRet<kstd::string> ProcFSContent::vmspace(pid_t pid) {
	auto proc = TRY(TaskManager::process_for_pid(pid));
	kstd::string string;
	char numbuf[12];
	proc->vm_space()->iterate_regions([&] (VMRegion* region) -> kstd::IterationAction {
		string += "0x";
		itoa(region->start(), numbuf, 16);
		string += numbuf;

		string += "\t0x";
		itoa(region->size(), numbuf, 16);
		string += numbuf;

		string += "\t0x";
		itoa(region->object_start(), numbuf, 16);
		string += numbuf;

		string += "\t";
		auto prot = region->prot();
		string += prot.read ? "r" : "-";
		string += prot.write ? "w" : "-";
		string += prot.execute ? "x" : "-";

		auto object = region->object();

		if (object->is_anonymous()) {
			auto anon_object = kstd::static_pointer_cast<AnonymousVMObject>(object);
			string += anon_object->is_shared() ? "s" : "-";
			string += " A";
		} else if (object->is_inode()) {
			auto inode_object = kstd::static_pointer_cast<InodeVMObject>(object);
			string += (inode_object->type() == InodeVMObject::Type::Shared) ? "s" : "-";
			string += " I";
		} else {
			ASSERT(false);
		}

		string += "\t";
		if (object->is_anonymous())
			string += "[";
		string += object->name();
		if (object->is_anonymous())
			string += "]";
		string += "\n";

		return kstd::IterationAction::Continue;
	});
	return string;
}

#ifdef DEBUG
extern kstd::map<Lock*, int> g_lock_registry;
#endif

ResultRet<kstd::string> ProcFSContent::lock_info() {
#ifdef DEBUG
	kstd::string string;
	char numbuf[32];
	for (auto pair : g_lock_registry) {
		if (pair.first->contest_count() == 0)
			continue;
		string += pair.first->name();
		string += "\t";
		lltoa(pair.first->contest_count(), numbuf, 10);
		string += numbuf;
		string += "\n";
	}
	return string;
#else
	return kstd::string("");
#endif
}
