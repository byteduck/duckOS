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

    Copyright (c) Byteduck 2016-2020. All rights reserved.
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
#include <kernel/memory/PageDirectory.h>

TSS TaskManager::tss;
SpinLock TaskManager::lock;

kstd::shared_ptr<Thread> cur_thread;
Process* kidle_process;
kstd::vector<Process*>* processes = nullptr;

kstd::queue<kstd::shared_ptr<Thread>>* thread_queue = nullptr;

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
		return -ENOENT;
	for(int i = 0; i < processes->size(); i++) {
		auto cur = processes->at(i);
		if(cur->pid() == pid && cur->state() != Process::DEAD)
			return processes->at(i);
	}
	return -ENOENT;
}

ResultRet<Process*> TaskManager::process_for_pgid(pid_t pgid, pid_t excl){
	if(!pgid)
		return -ENOENT;
	for(int i = 0; i < processes->size(); i++) {
		auto cur = processes->at(i);
		if(cur->pgid() == pgid && cur->pid() != excl && cur->state() != Process::DEAD)
			return processes->at(i);
	}
	return -ENOENT;
}

ResultRet<Process*> TaskManager::process_for_ppid(pid_t ppid, pid_t excl){
	if(!ppid)
		return -ENOENT;
	for(int i = 0; i < processes->size(); i++) {
		auto cur = processes->at(i);
		if(cur->ppid() == ppid && cur->pid() != excl && cur->state() != Process::DEAD)
			return processes->at(i);
	}
	return -ENOENT;
}

ResultRet<Process*> TaskManager::process_for_sid(pid_t sid, pid_t excl){
	if(!sid)
		return -ENOENT;
	for(int i = 0; i < processes->size(); i++) {
		auto cur = processes->at(i);
		if(cur->sid() == sid && cur->pid() != excl && cur->state() != Process::DEAD)
			return processes->at(i);
	}
	return -ENOENT;
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

bool& TaskManager::enabled(){
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
	lock = SpinLock();

	processes = new kstd::vector<Process*>();
	thread_queue = new kstd::queue<kstd::shared_ptr<Thread>>();

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

kstd::shared_ptr<Thread>& TaskManager::current_thread() {
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

void TaskManager::queue_thread(const kstd::shared_ptr<Thread>& thread) {
	TaskManager::Disabler disabler;
	if(!thread) {
		printf("[TaskManager] WARN: Tried queueing null thread!\n");
		return;
	}
	if(thread == kidle_process->main_thread()) {
		printf("[TaskManager] WARN: Tried queuing kidle thread!\n");
		return;
	}
	if(thread->state() != Thread::ALIVE) {
		printf("[TaskManager] WARN: Tried queuing blocked thread!\n");
		return;
	}
	thread_queue->push_back(thread);
}

void TaskManager::notify_current(uint32_t sig){
	cur_thread->process()->kill(sig);
}

kstd::shared_ptr<Thread> TaskManager::next_thread() {
	while(!thread_queue->empty() && thread_queue->front()->state() != Thread::ALIVE)
		thread_queue->pop_front();

	if(thread_queue->empty()) {
		if(cur_thread->state() == Thread::ALIVE)
			return cur_thread;
		else
			return kidle_process->main_thread();
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


void TaskManager::preempt(){
	if(preempting) return;
	if(!tasking_enabled) return;
	preempting = true;

	static bool prev_alive = false; // Whether or not there was an alive process last preemption
	bool any_alive = false;

	//Handle pending signals, cleanup dead processes, and release zombie processes' resources
	for(int i = 0; i < processes->size(); i++) {
		auto current = processes->at(i);
		switch(current->state()) {
			case Process::ALIVE: {
				current->handle_pending_signal();
				auto& threads = current->threads();
				//Evaluate if any of the process's threads are alive or need unblocking
				for (int j = 0; j < threads.size(); j++) {
					auto thread = threads[j];
					if(!thread)
						continue;
					if (thread->state() == Thread::BLOCKED) {
						if (thread->should_unblock()) {
							any_alive = true;
							thread->unblock();
							if(cur_thread != thread)
								queue_thread(thread);
						}
					} else if (thread->state() == Thread::ALIVE) {
						any_alive = true;
					}
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

	/*
	 * If it's time to switch, switch.
	 * A task switch will occur if the current process's quantum is up, if there were previously no running processes
	 * and one has become ready, and only if there is more than one running process.
	 */
	bool force_switch = !prev_alive && any_alive;
	prev_alive = any_alive;
	if(quantum_counter == 0 || force_switch) {
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
		ASSERT(cur_thread->state() == Thread::ALIVE);
		if(should_preempt) {
			if(old_thread != kidle_process->main_thread() && old_thread->state() == Thread::ALIVE)
				queue_thread(old_thread);

			//In case this thread is being destroyed, we don't want the reference in old_thread to keep it around
			old_thread = kstd::shared_ptr<Thread>(nullptr);

			asm volatile("fxsave %0" : "=m"(cur_thread->fpu_state));
			preempt_asm(old_esp, new_esp, cur_thread->process()->page_directory()->entries_physaddr());
			asm volatile("fxrstor %0" ::"m"(cur_thread->fpu_state));
		}
	} else {
		ASSERT(cur_thread->state() == Thread::ALIVE);
		quantum_counter--;
		preempting = false;
	}
}

TaskManager::Disabler::Disabler(): _enabled(enabled()) {
	enabled() = false;
}

TaskManager::Disabler::~Disabler() {
	enabled() = _enabled;
}
