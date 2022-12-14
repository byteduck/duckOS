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

#include <kernel/tasking/TaskManager.h>
#include <kernel/kstd/kstddef.h>
#include <kernel/kstd/kstdio.h>
#include <kernel/kmain.h>
#include <kernel/interrupt/irq.h>
#include <kernel/filesystem/procfs/ProcFS.h>
#include "TSS.h"
#include "Process.h"
#include "Thread.h"
#include "../interrupt/interrupt.h"
#include <kernel/memory/PageDirectory.h>
#include <kernel/kstd/KLog.h>

TSS TaskManager::tss;
SpinLock TaskManager::lock;

kstd::Arc<Thread> cur_thread;
Process* kidle_process;
kstd::vector<Process*>* processes = nullptr;

kstd::queue<kstd::Arc<Thread>>* thread_queue = nullptr;

uint32_t __cpid__ = 0;
bool tasking_enabled = false;
bool yield_async = false;
bool preempting = false;
static uint8_t quantum_counter = 0;

void kidle(){
	tasking_enabled = true;
	while(1) {
		asm volatile("hlt");
	}
}

ResultRet<Process*> TaskManager::process_for_pid(pid_t pid){
	if(!pid)
		return Result(-ENOENT);
	for(int i = 0; i < processes->size(); i++) {
		auto cur = processes->at(i);
		if(cur->pid() == pid && cur->state() != Process::DEAD)
			return processes->at(i);
	}
	return Result(-ENOENT);
}

ResultRet<Process*> TaskManager::process_for_pgid(pid_t pgid, pid_t excl){
	if(!pgid)
		return Result(-ENOENT);
	for(int i = 0; i < processes->size(); i++) {
		auto cur = processes->at(i);
		if(cur->pgid() == pgid && cur->pid() != excl && cur->state() != Process::DEAD)
			return processes->at(i);
	}
	return Result(-ENOENT);
}

ResultRet<Process*> TaskManager::process_for_ppid(pid_t ppid, pid_t excl){
	if(!ppid)
		return Result(-ENOENT);
	for(int i = 0; i < processes->size(); i++) {
		auto cur = processes->at(i);
		if(cur->ppid() == ppid && cur->pid() != excl && cur->state() != Process::DEAD)
			return processes->at(i);
	}
	return Result(-ENOENT);
}

ResultRet<Process*> TaskManager::process_for_sid(pid_t sid, pid_t excl){
	if(!sid)
		return Result(-ENOENT);
	for(int i = 0; i < processes->size(); i++) {
		auto cur = processes->at(i);
		if(cur->sid() == sid && cur->pid() != excl && cur->state() != Process::DEAD)
			return processes->at(i);
	}
	return Result(-ENOENT);
}

void TaskManager::kill_pgid(pid_t pgid, int sig) {
	if(!pgid)
		return;
	for(int i = 0; i < processes->size(); i++) {
		auto cur = processes->at(i);
		if(cur->pgid() == pgid) {
			cur->kill(sig);
			return;
		}
	}
}

void TaskManager::reparent_orphans(Process* proc) {
	for(int i = 0; i < processes->size(); i++) {
		auto cur = processes->at(i);
		if(cur->ppid() == proc->pid()) {
			cur->set_ppid(1);
		}
	}
}

bool TaskManager::enabled(){
	return tasking_enabled;
}

bool TaskManager::is_idle() {
	if(!kidle_process)
		return true;
	return cur_thread == kidle_process->main_thread();
}

bool TaskManager::is_preempting() {
	return preempting;
}

pid_t TaskManager::get_new_pid(){
	return __cpid__++;
}

void TaskManager::init(){
	KLog::dbg("TaskManager", "Initializing tasking...");

	lock = SpinLock();

	thread_queue = new kstd::queue<kstd::Arc<Thread>>();
	processes = new kstd::vector<Process*>();

	//Create kidle process
	kidle_process = Process::create_kernel("kidle", kidle);
	processes->push_back(kidle_process);

	//Create kinit process
	auto kinit_process = Process::create_kernel("kinit", kmain_late);
	processes->push_back(kinit_process);

	//Add kinit thread to queue
	queue_thread(kinit_process->main_thread());

	//Preempt
	cur_thread = kidle_process->main_thread();
	preempt_init_asm(cur_thread->registers.esp);
}

kstd::vector<Process*>* TaskManager::process_list() {
	return processes;
}

kstd::Arc<Thread>& TaskManager::current_thread() {
	return cur_thread;
}

Process* TaskManager::current_process() {
	return cur_thread->process();
}

int TaskManager::add_process(Process* proc){
	tasking_enabled = false;
	ProcFS::inst().proc_add(proc);
	processes->push_back(proc);
	auto& threads = proc->threads();
	for(int i = 0; i < threads.size(); i++)
		queue_thread(threads[i]);
	tasking_enabled = true;
	return proc->pid();
}

void TaskManager::queue_thread(const kstd::Arc<Thread>& thread) {
	Interrupt::Disabler disabler;
	if(!thread) {
		KLog::warn("TaskManager", "Tried queueing null thread!");
		return;
	}
	if(thread == kidle_process->main_thread()) {
		KLog::warn("TaskManager", "Tried queuing kidle thread!");
		return;
	}
	if(thread->state() != Thread::ALIVE) {
		KLog::warn("TaskManager", "Tried queuing %s thread!", thread->state_name());
		return;
	}
	thread_queue->push_back(thread);
}

void TaskManager::notify_current(uint32_t sig){
	cur_thread->process()->kill(sig);
}

kstd::Arc<Thread> TaskManager::next_thread() {
	while(!thread_queue->empty() && !thread_queue->front()->can_be_run())
		thread_queue->pop_front();

	if(thread_queue->empty()) {
		if(cur_thread->can_be_run()) {
			return cur_thread;
		} else if(kidle_process->main_thread()->state() != Thread::ALIVE) {
			PANIC("KTHREAD_DEADLOCK",
				  "The kernel thread is blocked, and no other thread is available to switch to.");
		} else {
			return kidle_process->main_thread();
		}
	}

	return thread_queue->pop_front();
}

bool TaskManager::yield() {
	ASSERT(!preempting);
	quantum_counter = 0;
	if(Interrupt::in_irq()) {
		// We can't yield in an interrupt. Instead, we'll yield immediately after we exit the interrupt
		yield_async = true;
		return false;
	} else {
		asm volatile("int $0x81");
		return true;
	}
}

bool TaskManager::yield_if_not_preempting() {
	if(!preempting)
		return yield();
	return true;
}

bool TaskManager::yield_if_idle() {
	if(!kidle_process)
		return false;
	if(cur_thread->process() == kidle_process)
		return yield();
	return false;
}

void TaskManager::do_yield_async() {
	if(yield_async) {
		yield_async = false;
		asm volatile("int $0x81");
	}
}

#pragma GCC push_options
#pragma GCC optimize ("O0")
void TaskManager::preempt(){
	if(!tasking_enabled) return;

	cur_thread->enter_critical();

	static bool prev_alive = false; // Whether or not there was an alive process last preemption
	bool any_alive = false;

	/*
	 * Try unblocking threads that are blocked
	 */
	for(int i = 0; i < processes->size(); i++) {
		auto current = processes->at(i);
		if(current->state() == Process::ALIVE) {
			auto& threads = current->threads();
			for(int j = 0; j < threads.size(); j++) {
				auto& thread = threads[j];
				if(!thread)
					continue;
				if(thread->state() == Thread::BLOCKED) {
					if(thread->should_unblock()) {
						thread->unblock();
						if(cur_thread != thread)
							queue_thread(thread);
					}
				}
			}
		}
	}

	/*
	 * Only update process/thread states if the thread we're switching from isn't already blocked, as doing so may
	 * try to block the thread again (ie acquiring the liballoc lock)
	 */
	if(cur_thread->can_be_run()) {
		LOCK(lock);
		//Handle pending signals, cleanup dead processes, and release zombie processes' resources
		for(int i = 0; i < processes->size(); i++) {
			auto current = processes->at(i);
			switch(current->state()) {
				case Process::ALIVE: {
					current->handle_pending_signal();
					auto& threads = current->threads();

					//Evaluate if any of the process's threads are alive or need unblocking
					for(int j = 0; j < threads.size(); j++) {
						auto thread = threads[j];
						if(!thread)
							continue;
						if(thread->state() == Thread::ALIVE)
							any_alive = true;
					}

					break;
				}
				case Process::DEAD: {
					if(cur_thread->process() != current) {
						current->free_resources();
						ProcFS::inst().proc_remove(current);
						processes->erase(i);
						delete current;
						i--;
					}
					break;
				}
				case Process::ZOMBIE:
					break;
				default:
					PANIC("PROC_INVALID_STATE", "A process had an invalid state (%d).", current->state());
			}
		}
	}

	preempting = true;

	/*
	 * If it's time to switch, switch.
	 * A task switch will occur if the current process's quantum is up, if there were previously no running processes
	 * and one has become ready, and only if there is more than one running process.
	 */
	//Pick a new process and decrease the quantum counter
	auto old_thread = cur_thread;
	cur_thread = next_thread();
	quantum_counter = 1; //Every process has a quantum of 1 for now

	bool should_preempt = old_thread != cur_thread;

	//If we were just in a signal handler, don't save the esp to old_proc->registers
	unsigned int* old_esp;
	unsigned int dummy_esp;
	if(!old_thread) {
		old_esp = &dummy_esp;
	} if(old_thread->in_signal_handler()) {
		old_esp = &old_thread->signal_registers.esp;
	} else {
		old_esp = &old_thread->registers.esp;
	}

	//If we just finished handling a signal, set in_signal_handler to false.
	if(old_thread && old_thread->just_finished_signal()) {
		should_preempt = true;
		old_thread->just_finished_signal() = false;
		old_thread->in_signal_handler() = false;
	}

	//If we're about to start handling a signal, set in_signal_handler to true.
	if(cur_thread->ready_to_handle_signal()) {
		should_preempt = true;
		cur_thread->in_signal_handler() = true;
		cur_thread->ready_to_handle_signal() = false;
	}

	//If we're switching to a process in a signal handler, use the esp from signal_registers
	unsigned int* new_esp;
	if(cur_thread->in_signal_handler()){
		new_esp = &cur_thread->signal_registers.esp;
		tss.esp0 = (size_t) cur_thread->signal_stack_top();
	} else {
		new_esp = &cur_thread->registers.esp;
		tss.esp0 = (size_t) cur_thread->kernel_stack_top();
	}

	if(should_preempt)
		cur_thread->process()->set_last_active_thread(cur_thread->tid());

	//Switch tasks.
	preempting = false;
	if(!cur_thread->can_be_run())
		PANIC("INVALID_CONTEXT_SWITCH", "Tried to switch to thread %d of PID %d in state %d", cur_thread->tid(), cur_thread->process()->pid(), cur_thread->state());
	if(should_preempt) {
		if(old_thread != kidle_process->main_thread() && old_thread->can_be_run())
			queue_thread(old_thread);

		old_thread->leave_critical();

		//In case this thread is being destroyed, we don't want the reference in old_thread to keep it around
		old_thread = kstd::Arc<Thread>(nullptr);

		asm volatile("fxsave %0" : "=m"(cur_thread->fpu_state));
		preempt_asm(old_esp, new_esp, cur_thread->process()->page_directory()->entries_physaddr());
		asm volatile("fxrstor %0" ::"m"(cur_thread->fpu_state));
	} else {
		old_thread->leave_critical();
	}
}
#pragma GCC pop_options
