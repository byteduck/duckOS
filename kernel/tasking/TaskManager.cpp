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
#include <kernel/kmain.h>
#include <kernel/interrupt/irq.h>
#include <kernel/filesystem/procfs/ProcFS.h>
#include "TSS.h"
#include "Process.h"
#include "Thread.h"
#include "Reaper.h"
#include <kernel/kstd/KLog.h>

TSS TaskManager::tss;
SpinLock TaskManager::g_tasking_lock;
SpinLock TaskManager::g_process_lock;

kstd::Arc<Thread> cur_thread;
Process* kernel_process;
kstd::vector<Process*>* processes = nullptr;
Thread* TaskManager::g_next_thread;

uint32_t __cpid__ = 0;
bool tasking_enabled = false;
bool yield_async = false;
bool preempting = false;
static uint8_t quantum_counter = 0;

void kidle(){
	tasking_enabled = true;
	TaskManager::yield();
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

void TaskManager::reparent_orphans(Process* dead) {
	CRITICAL_LOCK(g_tasking_lock);
	for(auto process : *processes)
		if(process->ppid() == dead->pid())
			process->set_ppid(1);
}

bool TaskManager::enabled(){
	return tasking_enabled;
}

bool TaskManager::is_idle() {
	if(!kernel_process)
		return true;
	return cur_thread == kernel_process->main_thread();
}

bool TaskManager::is_preempting() {
	return preempting;
}

pid_t TaskManager::get_new_pid(){
	return __cpid__++;
}

void TaskManager::init(){
	KLog::dbg("TaskManager", "Initializing tasking...");
	g_tasking_lock.acquire();

	processes = new kstd::vector<Process*>();

	//Create kernel process
	kernel_process = Process::create_kernel("[kernel]", kidle);
	processes->push_back(kernel_process);

	//Create kinit process
	auto kinit_process = Process::create_kernel("[kinit]", kmain_late);
	processes->push_back(kinit_process);
	queue_thread(kinit_process->main_thread());

	//Create kernel threads
	kernel_process->spawn_kernel_thread(kreaper_entry);

	//Preempt
	cur_thread = kernel_process->main_thread();
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
	g_process_lock.acquire();
	ProcFS::inst().proc_add(proc);
	processes->push_back(proc);
	g_process_lock.release();

	CRITICAL_LOCK(g_tasking_lock);
	auto& threads = proc->threads();
	for(const auto& thread : threads)
		queue_thread(thread);
	return proc->pid();
}

void TaskManager::remove_process(Process* proc) {
	LOCK(g_process_lock);
	for(size_t i = 0; i < processes->size(); i++) {
		if(processes->at(i) == proc) {
			processes->erase(i);
			return;
		}
	}
}

void TaskManager::queue_thread(const kstd::Arc<Thread>& thread) {
	ASSERT(g_tasking_lock.held_by_current_thread());

	if(!thread) {
		KLog::warn("TaskManager", "Tried queueing null thread!");
		return;
	}
	if(thread == kernel_process->main_thread()) {
		KLog::warn("TaskManager", "Tried queuing kidle thread!");
		return;
	}
	if(thread->state() != Thread::ALIVE) {
		KLog::warn("TaskManager", "Tried queuing %s thread!", thread->state_name());
		return;
	}

	if(g_next_thread)
		g_next_thread->enqueue_thread(thread.get());
	else
		g_next_thread = thread.get();
}

void TaskManager::notify_current(uint32_t sig){
	cur_thread->process()->kill(sig);
}

kstd::Arc<Thread> TaskManager::pick_next_thread() {
	ASSERT(g_tasking_lock.held_by_current_thread());

	// Make sure the next thread is in a runnable state
	while(g_next_thread && !g_next_thread->can_be_run())
		g_next_thread = g_next_thread->next_thread();

	// If we don't have a next thread to run, either continue running the current thread or run kidle
	if(!g_next_thread) {
		if(cur_thread->can_be_run()) {
			return cur_thread;
		} else if(kernel_process->main_thread()->state() != Thread::ALIVE) {
			PANIC("KTHREAD_DEADLOCK", "The kernel idle thread is blocked!");
		} else {
			return kernel_process->main_thread();
		}
	}

	auto next = g_next_thread->self();
	g_next_thread = g_next_thread->next_thread();
	return next;
}

bool TaskManager::yield() {
	ASSERT(!preempting);
	quantum_counter = 0;
	if(Interrupt::in_irq()) {
		// We can't yield in an interrupt. Instead, we'll yield immediately after we exit the interrupt
		yield_async = true;
		return false;
	} else {
		preempt();
		return true;
	}
}

bool TaskManager::yield_if_not_preempting() {
	if(!preempting)
		return yield();
	return true;
}

bool TaskManager::yield_if_idle() {
	if(!kernel_process)
		return false;
	if(cur_thread == kernel_process->main_thread())
		return yield();
	return false;
}

void TaskManager::do_yield_async() {
	if(yield_async) {
		yield_async = false;
		preempt();
	}
}

void TaskManager::tick() {
	ASSERT(Interrupt::in_irq());
	yield();
}

Atomic<int, MemoryOrder::SeqCst> g_critical_count = 0;

void TaskManager::enter_critical() {
	asm volatile("cli");
	g_critical_count.add(1);
}

void TaskManager::leave_critical() {
	ASSERT(g_critical_count.load() > 0);
	if(g_critical_count.sub(1) == 1)
		asm volatile("sti");
}

bool TaskManager::in_critical() {
	return g_critical_count.load();
}

#pragma GCC push_options
#pragma GCC optimize ("O0")
void TaskManager::preempt(){
	if(!tasking_enabled)
		return;
	ASSERT(!g_critical_count.load());

	g_tasking_lock.acquire_and_enter_critical();
	cur_thread->enter_critical();
	preempting = true;

	// Try unblocking threads that are blocked
	if(g_process_lock.try_acquire()) {
		for(auto& process : *processes) {
			if(process->state() != Process::ALIVE)
				break;
			for(auto& thread : process->threads()) {
				if(!thread)
					continue;
				if(thread->state() == Thread::BLOCKED && thread->should_unblock())
					thread->unblock();
			}
		}
		g_process_lock.release();
	}

	// Pick a new thread
	auto old_thread = cur_thread;
	auto next_thread = pick_next_thread();
	quantum_counter = 1; //Every process has a quantum of 1 for now

	bool should_preempt = old_thread != next_thread;

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
	if(next_thread->ready_to_handle_signal()) {
		should_preempt = true;
		next_thread->in_signal_handler() = true;
		next_thread->ready_to_handle_signal() = false;
	}

	//If we're switching to a process in a signal handler, use the esp from signal_registers
	unsigned int* new_esp;
	if(next_thread->in_signal_handler()) {
		new_esp = &next_thread->signal_registers.esp;
		tss.esp0 = (size_t) next_thread->signal_stack_top();
	} else {
		new_esp = &next_thread->registers.esp;
		tss.esp0 = (size_t) next_thread->kernel_stack_top();
	}

	if(should_preempt)
		next_thread->process()->set_last_active_thread(next_thread->tid());

	// Switch context.
	preempting = false;
	if(!next_thread->can_be_run())
		PANIC("INVALID_CONTEXT_SWITCH", "Tried to switch to thread %d of PID %d in state %d", next_thread->tid(), next_thread->process()->pid(), next_thread->state());
	if(should_preempt) {
		// If we can run the old thread, re-queue it after we preempt
		if(old_thread != kernel_process->main_thread() && old_thread->can_be_run())
			queue_thread(old_thread);

		cur_thread = next_thread;
		next_thread.reset();
		old_thread.reset();

		asm volatile("fxsave %0" : "=m"(cur_thread->fpu_state));
		preempt_asm(old_esp, new_esp, cur_thread->process()->page_directory()->entries_physaddr());
		asm volatile("fxrstor %0" ::"m"(cur_thread->fpu_state));
	}

	preempt_finish();
}
#pragma GCC pop_options

void TaskManager::preempt_finish() {
	ASSERT(g_tasking_lock.count() == 1);
	g_tasking_lock.release();
	leave_critical();
	// Handle a pending signal.
	if(cur_thread != kernel_process->main_thread())
		cur_thread->process()->handle_pending_signal();
	cur_thread->leave_critical();
}