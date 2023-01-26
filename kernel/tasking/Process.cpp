/*
	This file is part of duckOS.

	duckOS is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	duckOS is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with duckOS.  If not, see <https://www.gnu.org/licenses/>.

	Copyright (c) Byteduck 2016-2021. All rights reserved.
*/

#include <kernel/filesystem/VFS.h>
#include <kernel/terminal/TTYDevice.h>
#include "Process.h"
#include "TaskManager.h"
#include "ELF.h"
#include "ProcessArgs.h"
#include "../syscall/syscall.h"
#include "../terminal/VirtualTTY.h"
#include "Thread.h"
#include "../kstd/KLog.h"
#include "../filesystem/procfs/ProcFS.h"

Process* Process::create_kernel(const kstd::string& name, void (*func)()){
	ProcessArgs args = ProcessArgs(kstd::Arc<LinkedInode>(nullptr));
	auto* ret = new Process(name, (size_t)func, true, &args, TaskManager::get_new_pid(), 1);
	return ret->_self_ptr;
}

ResultRet<Process*> Process::create_user(const kstd::string& executable_loc, User& file_open_user, ProcessArgs* args, pid_t pid, pid_t parent) {
	//Open the executable
	auto fd_or_error = VFS::inst().open((kstd::string&) executable_loc, O_RDONLY, 0, file_open_user, args->working_dir);
	if(fd_or_error.is_error())
		return fd_or_error.result();
	auto fd = fd_or_error.value();

	//Read info
	auto info_or_err = ELF::read_info(fd, file_open_user);
	if(info_or_err.is_error())
		return info_or_err.result();
	auto info = info_or_err.value();

	//If there's an interpreter, we need to change the arguments accordingly
	if(info.interpreter.length()) {
		KLog::dbg("Process", "Executable %s requesting interpreter %s", executable_loc.c_str(), info.interpreter.c_str());

		//Get the full path of the program we're trying to run
		auto resolv = VFS::inst().resolve_path((kstd::string&) executable_loc, args->working_dir, file_open_user);
		if(resolv.is_error())
			return resolv.result();

		//Add the interpreter + the full path of the program to the args
		auto new_argv = kstd::vector<kstd::string>();
		new_argv.push_back(info.interpreter);
		new_argv.push_back(resolv.value()->get_full_path());

		//Add the old arguments to the new argv and replace the argv
		for(size_t i = 0; i < args->argv.size(); i++)
			new_argv.push_back(args->argv[i]);
		args->argv = new_argv;
	}

	//Create the process
	auto* proc = new Process(VFS::path_base(executable_loc), info.header->program_entry_position, false, args, pid, parent);
	proc->_exe = executable_loc;

	//Add the regions into the process's vm regions
	auto regions = TRY(ELF::load_sections(*info.fd, info.segments, proc->_vm_space));
	for(const auto& region : regions)
		proc->_vm_regions.push_back(region);

	proc->recalculate_pmem_total();

	return proc->_self_ptr;
}

pid_t Process::pid() {
	return _pid;
}

pid_t Process::pgid() {
	return _pgid;
}

pid_t Process::ppid() {
	return _ppid;
}

void Process::set_ppid(pid_t ppid) {
	_ppid = ppid;
}

pid_t Process::sid() {
	return _sid;
}

User Process::user() {
	return _user;
}

kstd::string Process::name(){
	return _name;
}

kstd::string Process::exe() {
	return _exe;
}

kstd::Arc<LinkedInode> Process::cwd() {
	return _cwd;
}

void Process::set_tty(kstd::Arc<TTYDevice> tty) {
	_tty = tty;
}

Process::State Process::state() {
	return _state;
}

int Process::all_threads_state() {
	if(_state != ALIVE)
		return _state;
	LOCK(_thread_lock);
	for(auto& tid : _tids)
		if(_threads[tid]->state() == Thread::ALIVE)
			return ALIVE;
	return Thread::BLOCKED;
}

int Process::exit_status() {
	return _exit_status;
}

bool Process::is_kernel_mode() {
	return _kernel_mode;
}

tid_t Process::last_active_thread() {
	return _last_active_thread;
}

void Process::set_last_active_thread(tid_t tid) {
	_last_active_thread = tid;
}

kstd::Arc<Thread> Process::spawn_kernel_thread(void (*entry)()) {
	ProcessArgs args = ProcessArgs(kstd::Arc<LinkedInode>(nullptr));
	auto thread = kstd::make_shared<Thread>(_self_ptr, TaskManager::get_new_pid(), (size_t) entry, &args);
	insert_thread(thread);
	ASSERT(TaskManager::g_tasking_lock.held_by_current_thread());
	TaskManager::queue_thread(thread);
	return thread;
}

const kstd::vector<tid_t>& Process::threads() {
	return _tids;
}

kstd::Arc<Thread> Process::get_thread(tid_t tid) {
	auto node = _threads.find_node(tid);
	if(node)
		return node->data.second;
	return {};
}

Process::Process(const kstd::string& name, size_t entry_point, bool kernel, ProcessArgs* args, pid_t pid, pid_t ppid):
		_user(User::root()),
		_name(name),
		_pid(pid),
		_kernel_mode(kernel),
		_ppid(_pid > 1 ? ppid : 0),
		_state(ALIVE),
		_self_ptr(this)
{
	if(!kernel) {
		auto ttydesc = kstd::make_shared<FileDescriptor>(VirtualTTY::current_tty());
		ttydesc->set_owner(_self_ptr);
		ttydesc->set_options(O_RDWR);
		_file_descriptors.push_back(ttydesc);
		_file_descriptors.push_back(ttydesc);
		_file_descriptors.push_back(ttydesc);
		_cwd = args->working_dir;

		//Make new page directory
		_page_directory = kstd::make_shared<PageDirectory>();
		_vm_space = kstd::make_shared<VMSpace>(PAGE_SIZE, HIGHER_HALF - PAGE_SIZE, *_page_directory);
	}

	//Create the main thread
	auto* main_thread = new Thread(_self_ptr, _pid, entry_point, args);
	insert_thread(kstd::Arc<Thread>(main_thread));
}

Process::Process(Process *to_fork, Registers &regs): _user(to_fork->_user), _self_ptr(this) {
	if(to_fork->_kernel_mode)
		PANIC("KRNL_PROCESS_FORK", "Kernel processes cannot be forked.");

	// TODO
	_name = to_fork->_name;
	_pid = TaskManager::get_new_pid();
	_kernel_mode = false;
	_cwd = to_fork->_cwd;
	_ppid = to_fork->_pid;
	_sid = to_fork->_sid;
	_pgid = to_fork->_pgid;
	_umask = to_fork->_umask;
	_tty = to_fork->_tty;
	m_used_pmem = to_fork->m_used_pmem;
	m_used_shmem = to_fork->m_used_shmem;
	_state = ALIVE;

	//TODO: Prevent thread race condition when copying signal handlers/file descriptors
	//Copy signal handlers
	for(int i = 0; i < 32; i++)
		signal_actions[i] = to_fork->signal_actions[i];

	//Copy file descriptors
	_file_descriptors.resize(to_fork->_file_descriptors.size());
	for(size_t i = 0; i < to_fork->_file_descriptors.size(); i++) {
		if(to_fork->_file_descriptors[i]) {
			_file_descriptors[i] = kstd::make_shared<FileDescriptor>(*to_fork->_file_descriptors[i]);
			_file_descriptors[i]->set_owner(_self_ptr);
		}
	}

	// Create page directory and fork the old one
	/* TODO: We're probably leaking thread stack regions here, since they'll be put into _vm_regions rather than
	 * Thread::_stack_region (they will be cleaned up once the process dies / exec()s, though).
	 */
	_page_directory = kstd::make_shared<PageDirectory>();
	_vm_space = to_fork->_vm_space->fork(*_page_directory, _vm_regions);

	//Create the main thread
	auto* main_thread = new Thread(_self_ptr, _pid, regs);
	insert_thread(kstd::Arc<Thread>(main_thread));
}

Process::~Process() {
	TaskManager::remove_process(this);
}

void Process::kill(int signal) {
	if(signal >= 0 && signal <= 32) {
		Signal::SignalSeverity severity = Signal::signal_severities[signal];

		//Print signal if unhandled and fatal
		if(severity == Signal::FATAL && !signal_actions[signal].action) {
			KLog::warn("Process", "PID %d exiting with fatal signal %s", _pid, Signal::signal_names[signal]);
		}

		if(severity >= Signal::KILL && !signal_actions[signal].action) {
			//If the signal has no handler and is KILL or FATAL, then kill all threads
			TaskManager::current_thread()->enter_critical();
			for(auto tid : _tids)
				get_thread(tid)->kill();
			TaskManager::current_thread()->leave_critical();
		} else if(signal_actions[signal].action) {
			// We have a signal handler for this.
			if(TaskManager::current_thread()->process() == this) {
				// We are a thread from this process - try to handle the signal immediately.
				if(!TaskManager::current_thread()->call_signal_handler(signal)) {
					// We couldn't handle it - push it back to the queue.
					LOCK(m_signal_lock);
					pending_signals.push_back(signal);
				}
			} else {
				// We are a thread from another process. Interrupt a thread if needed and queue the signal.
				ScopedLocker thread_locker(_thread_lock);
				ScopedLocker signal_locker(m_signal_lock);
				pending_signals.push_back(signal);
				TaskManager::ScopedCritical critical;
				for(auto& tid : _tids) {
					auto thread = get_thread(tid);
					if(thread->state() == Thread::ALIVE)
						break;
					if(thread->is_blocked() && thread->_blocker->can_be_interrupted()) {
						thread->_blocker->interrupt();
						thread->unblock();
						break;
					}
				}
			}
		}
	}
}

void Process::handle_pending_signal() {
	// We have to enter critical while holding the lock, or else if the process has multiple threads
	m_signal_lock.acquire_and_enter_critical();
	if(!pending_signals.empty()) {
		int sig = pending_signals.pop_front();
		m_signal_lock.release();
		TaskManager::leave_critical();
		kill(sig);
	} else {
		m_signal_lock.release();
		TaskManager::leave_critical();
	}
}

bool Process::has_pending_signals() {
	return !pending_signals.empty();
}

PageDirectory* Process::page_directory() {
	if(is_kernel_mode())
		return &MemoryManager::inst().kernel_page_directory;
	else
		return _page_directory.get();
}

kstd::Arc<VMSpace> Process::vm_space() {
	return _vm_space;
}

ResultRet<kstd::Arc<VMRegion>> Process::map_object(kstd::Arc<VMObject> object, VMProt prot) {
	auto region = TRY(_vm_space->map_object(object, prot));
	_vm_regions.push_back(region);
	return region;
}

ResultRet<kstd::Arc<VMRegion>> Process::map_object(kstd::Arc<VMObject> object, VirtualAddress address, VMProt prot) {
	auto region = TRY(_vm_space->map_object(object, prot, VirtualRange { address, object->size() }));
	_vm_regions.push_back(region);
	return region;
}

size_t Process::used_pmem() const {
	return m_used_pmem;
}

size_t Process::used_vmem() const {
	return _vm_space->used();
}

size_t Process::used_shmem() const {
	return m_used_shmem;
}

/************
 * SYSCALLS *
 ************/

void Process::check_ptr(const void *ptr, bool write) {
	auto region = _vm_space->get_region_containing((VirtualAddress) ptr);
	if(region.is_error()) {
		KLog::dbg("Process", "Pointer check at 0x%x failed for %s(%d): Not mapped", ptr, _name.c_str(), _pid);
		kill(SIGSEGV);
	}
	auto prot = region.value()->prot();
	if((!write && !prot.read) || (!(prot.write || prot.cow) && write)) {
		KLog::dbg("Process", "Pointer check at 0x%x failed for %s(%d): Insufficient permissions", ptr, _name.c_str(), _pid);
		kill(SIGSEGV);
	}
}

void Process::alert_thread_died(kstd::Arc<Thread> thread) {
	remove_thread(thread);

	// If all threads are dead, we are ready to die.
	if(_threads.size() == 0) {
		auto parent = TaskManager::process_for_pid(_ppid);
		if (!parent.is_error() && parent.value() != this) {
			parent.value()->kill(SIGCHLD);
		} else if(_pid == -1) {
			// We are a process that just exec()'d. Nothing to do here.
		} else {
			KLog::warn("Process", "Process %d died and did not have a parent for SIGCHLD!", _pid);
		}
		TaskManager::reparent_orphans(this);
		_state = ZOMBIE;
	}
}

void Process::recalculate_pmem_total() {
	if(_is_destroying || !_vm_space)
		return;
	m_used_pmem = _vm_space->calculate_regular_anonymous_total();
}

void Process::insert_thread(const kstd::Arc<Thread>& thread) {
	LOCK(_thread_lock);
	_threads[thread->_tid] = thread;
	_tids.push_back(thread->_tid);
}

void Process::remove_thread(const kstd::Arc<Thread>& thread) {
	LOCK(_thread_lock);
	_threads.erase(thread->_tid);
	for(size_t i = 0; i < _tids.size(); i++) {
		if(_tids[i] == thread->_tid) {
			_tids.erase(i);
			break;
		}
	}
}
