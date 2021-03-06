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
#include <kernel/kstddef.h>
#include <kernel/kstdio.h>
#include <kernel/kmain.h>
#include <kernel/interrupt/irq.h>
#include <kernel/pit.h>
#include <kernel/filesystem/procfs/ProcFS.h>

TSS TaskManager::tss;
SpinLock TaskManager::lock;

Process *current_proc = nullptr;
Process *kidle_proc;

uint32_t __cpid__ = 0;
bool tasking_enabled = false;
bool yield_async = false;

void kidle(){
	tasking_enabled = true;
	while(1) {
		asm volatile("sti");
		asm volatile("hlt");
	}
}

Process* TaskManager::process_for_pid(pid_t pid){
	Process *current = kidle_proc;
	do{
		if(current->pid() == pid && current->pid() && current->state != PROCESS_DEAD) return current;
		current = current->next;
	} while(current != kidle_proc);
	return (Process *) nullptr;
}

Process* TaskManager::process_for_pgid(pid_t pgid, pid_t excl){
	Process *current = kidle_proc;
	do{
		if(current->pgid() == pgid && current->pid() && current->pid() != excl && current->state != PROCESS_DEAD) return current;
		current = current->next;
	} while(current != kidle_proc);
	return (Process *) nullptr;
}

Process* TaskManager::process_for_ppid(pid_t ppid, pid_t excl){
	Process *current = kidle_proc;
	do{
		if(current->ppid() == ppid && current->pid() && current->pid() != excl && current->state != PROCESS_DEAD) return current;
		current = current->next;
	} while(current != kidle_proc);
	return (Process *) nullptr;
}

Process* TaskManager::process_for_sid(pid_t sid, pid_t excl){
	Process *current = kidle_proc;
	do{
		if(current->sid() == sid && current->pid() && current->pid() != excl && current->state != PROCESS_DEAD) return current;
		current = current->next;
	} while(current != kidle_proc);
	return (Process *) nullptr;
}

void TaskManager::kill_pgid(pid_t pgid, int sig) {
	Process *current = kidle_proc;
	do {
		if(current->pgid() == pgid)
			current->kill(sig);
		current = current->next;
	} while(current != kidle_proc);
}

void TaskManager::print_tasks(){
	Process *current = kidle_proc;
	printf("Running processes: ([PID] name state usermem)\n");
	do {
		printf("[%d] '%s' %d %dKiB\n", current->pid(), current->name().c_str(), current->state, current->ring == 3 ? current->page_directory->used_pmem() : 0);
		current = current->next;
	} while(current != kidle_proc);
}

void TaskManager::reparent_orphans(Process* proc) {
	tasking_enabled = false;
	auto* cchild = kidle_proc->next;
	do {
		if(cchild->ppid() == proc->pid())
			cchild->set_ppid(1);
		cchild = cchild->next;
	} while(cchild != kidle_proc);
	tasking_enabled = true;
}

bool& TaskManager::enabled(){
	return tasking_enabled;
}

bool TaskManager::is_idle() {
	return current_proc == kidle_proc;
}

pid_t TaskManager::get_new_pid(){
	return __cpid__++;
}

void TaskManager::init(){
	lock = SpinLock();

	//Create kidle process
	kidle_proc = Process::create_kernel("kidle", kidle);

	//Create kinit process
	auto* kinit_proc = Process::create_kernel("kinit", kmain_late);

	//Link processes together manually
	kidle_proc->next = kinit_proc;
	kidle_proc->prev = kinit_proc;
	kinit_proc->next = kidle_proc;
	kinit_proc->prev = kidle_proc;

	//Preempt
	current_proc = kidle_proc;
	preempt_init_asm(current_proc->registers.esp);
}

Process* TaskManager::current_process(){
	return current_proc;
}

uint32_t TaskManager::add_process(Process *p){
	ASSERT(tasking_enabled)
	tasking_enabled = false;
	ProcFS::inst().proc_add(p);
	p->next = current_proc->next;
	p->next->prev = p;
	p->prev = current_proc;
	current_proc->next = p;
	tasking_enabled = true;
	return p->pid();
}

void TaskManager::notify_current(uint32_t sig){
	current_proc->kill(sig);
}

Process *TaskManager::next_process() {
	Process* next_proc = current_proc->next;

	//Don't switch to a blocking/zombie process or the kidle proc
	while((next_proc->state != PROCESS_ALIVE || next_proc == kidle_proc) && next_proc != current_proc)
		next_proc = next_proc->next;

	//There are no processes alive, switch to kidle
	if(next_proc == current_proc && current_proc->state != PROCESS_ALIVE)
		next_proc = kidle_proc;

	return next_proc;
}


static uint8_t quantum_counter = 0;

bool TaskManager::yield() {
	quantum_counter = 0;
	if(Interrupt::in_interrupt()) {
		// We can't yield in an interrupt. Instead, we'll yield immediately after we exit the interrupt
		yield_async = true;
		return false;
	} else {
		preempt_now_asm();
		return true;
	}
}

bool TaskManager::yield_if_idle() {
	if(current_proc == kidle_proc)
		return yield();
	return false;
}

void TaskManager::do_yield_async() {
	if(yield_async) {
		yield_async = false;
		preempt_now_asm();
	}
}

void TaskManager::preempt(){
	if(!tasking_enabled) return;

	static bool prev_alive = false; // Whether or not there was an alive process last preemption
	bool any_alive = false;

	//Handle pending signals, cleanup dead processes, and release zombie processes' resources
	Process *current = kidle_proc->next;
	do {
		switch(current->state) {
			case PROCESS_BLOCKED:
				if(current->should_unblock()) {
					current->unblock();
					any_alive = true;
				}
				current->handle_pending_signal();
				current = current->next;
				break;
			case PROCESS_ALIVE:
				current->handle_pending_signal();
				current = current->next;
				any_alive = true;
				break;
			case PROCESS_DEAD:
			case PROCESS_ZOMBIE:
				current = current->next;
				break;
			default:
				PANIC("PROC_INVALID_STATE", "A process had an invalid state.", true);
		}
	} while(current != kidle_proc);

	/*
	 * If it's time to switch, switch.
	 * A task switch will occur if the current process's quantum is up, if there were previously no running processes
	 * and one has become ready, and only if there is more than one running process.
	 */
	bool force_switch = !prev_alive && any_alive;
	prev_alive = any_alive;
	if(quantum_counter == 0 || force_switch) {
		//Pick a new process and decrease the quantum counter
		auto old_proc = current_proc;
		current_proc = next_process();
		quantum_counter = current_proc->quantum - 1;

		bool should_preempt = old_proc != current_proc;

		//If we were just in a signal handler, don't save the esp to old_proc->registers
		unsigned int* old_esp;
		unsigned int dummy_esp;
		if(!old_proc) {
			old_esp = &dummy_esp;
		} if(old_proc->in_signal_handler()) {
			old_esp = &old_proc->signal_registers.esp;
		} else {
			old_esp = &old_proc->registers.esp;
		}

		//If we just finished handling a signal, set in_signal_handler to false.
		if(old_proc && old_proc->just_finished_signal()) {
			should_preempt = true;
			old_proc->just_finished_signal() = false;
			old_proc->in_signal_handler() = false;
		}

		//If we're about to start handling a signal, set in_signal_handler to true.
		if(current_proc->ready_to_handle_signal()) {
			should_preempt = true;
			current_proc->in_signal_handler() = true;
			current_proc->ready_to_handle_signal() = false;
		}

		//If we're switching to a process in a signal handler, use the esp from signal_registers
		unsigned int* new_esp;
		if(current_proc->in_signal_handler()){
			new_esp = &current_proc->signal_registers.esp;
			tss.esp0 = (size_t) current_proc->signal_stack_top();
		} else {
			new_esp = &current_proc->registers.esp;
			tss.esp0 = (size_t) current_proc->kernel_stack_top();
		}

		//Switch tasks.
		if(should_preempt)
			preempt_asm(old_esp, new_esp, current_proc->page_directory_loc);
	} else {
		quantum_counter--;
	}

	/*
	 * Clean up old, dead processes
	 */
	current = kidle_proc->next;
	do {
		if(current->state == PROCESS_DEAD) {
			current->prev->next = current->next;
			current->next->prev = current->prev;
			Process* to_delete = current;
			ProcFS::inst().proc_remove(to_delete);
			delete to_delete;
		}
		current = current->next;
	} while(current != kidle_proc);
}
