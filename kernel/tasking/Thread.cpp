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

#include "Thread.h"
#include "Process.h"
#include "ProcessArgs.h"
#include "Blocker.h"
#include "TaskManager.h"
#include "JoinBlocker.h"
#include <kernel/KernelMapper.h>
#include <kernel/interrupt/isr.h>
#include <kernel/memory/MemoryManager.h>
#include <kernel/memory/PageDirectory.h>
#include <kernel/memory/Stack.h>
#include <kernel/kstd/KLog.h>
#include <kernel/memory/SafePointer.h>
#include "../memory/AnonymousVMObject.h"

Thread::Thread(Process* process, tid_t tid, size_t entry_point, ProcessArgs* args): _tid(tid), _process(process) {
	//Create the kernel stack
	_kernel_stack_region = MM.alloc_kernel_region(THREAD_KERNEL_STACK_SIZE);
	kstd::Arc<VMRegion> mapped_user_stack_region;
	Stack user_stack(nullptr, 0);
	Stack kernel_stack((void*) _kernel_stack_region->end());

	if(!is_kernel_mode()) {
		auto do_create_stack = [&]() -> Result {
			auto stack_object = TRY(AnonymousVMObject::alloc(THREAD_STACK_SIZE));
			_stack_region = TRY(_process->_vm_space->map_stack(stack_object));
			mapped_user_stack_region = MM.map_object(stack_object);
			return Result(SUCCESS);
		};
		if (do_create_stack().is_error())
			PANIC("NEW_THREAD_STACK_ALLOC_FAIL", "Was unable to allocate virtual memory for a new thread's stack.");

		user_stack = Stack((void*) mapped_user_stack_region->end(), _stack_region->end());
	} else {
		user_stack = Stack((void*) _kernel_stack_region->end());
	}

	//Setup registers
	registers.eflags = 0x202;
	registers.cs = _process->_kernel_mode ? 0x8 : 0x1B;
	registers.eip = entry_point;
	registers.eax = 0;
	registers.ebx = 0;
	registers.ecx = 0;
	registers.edx = 0;
	registers.ebp = user_stack.real_stackptr();
	registers.edi = 0;
	registers.esi = 0;
	if(_process->_kernel_mode) {
		registers.ds = 0x10; // ds
		registers.es = 0x10; // es
		registers.fs = 0x10; // fs
		registers.gs = 0x10; // gs
	} else {
		registers.ds = 0x23; // ds
		registers.es = 0x23; // es
		registers.fs = 0x23; // fs
		registers.gs = 0x23; // gs
	}

	//Set up the user stack for the program arguments
	args->setup_stack(user_stack);

	//Setup the kernel stack with register states
	if(is_kernel_mode())
		setup_kernel_stack(kernel_stack, kernel_stack.real_stackptr(), registers);
	else
		setup_kernel_stack(kernel_stack, user_stack.real_stackptr(), registers);
}

Thread::Thread(Process* process, tid_t tid, Registers& regs): _process(process), _tid(tid), registers(regs) {
	//Allocate kernel stack
	_kernel_stack_region = MM.alloc_kernel_region(THREAD_KERNEL_STACK_SIZE);

	//Setup registers and stack
	registers.eax = 0; // fork() in child returns zero
	Stack stack((void*) (_kernel_stack_region->start() + _kernel_stack_region->size()));
	setup_kernel_stack(stack, regs.useresp, registers);
}

Thread::Thread(Process* process, tid_t tid, void* (*entry_func)(void* (*)(void*), void*), void* (* thread_func)(void*), void* arg): _tid(tid), _process(process) {
	//Create the kernel stack
	_kernel_stack_region = MM.alloc_kernel_region(THREAD_KERNEL_STACK_SIZE);
	kstd::Arc<VMRegion> mapped_user_stack_region;
	Stack user_stack(nullptr, 0);
	Stack kernel_stack((void*) (_kernel_stack_region->end()));

	if(!is_kernel_mode()) {
		auto do_create_stack = [&]() -> Result {
			auto stack_object = TRY(AnonymousVMObject::alloc(THREAD_STACK_SIZE));
			_stack_region = TRY(_process->_vm_space->map_stack(stack_object));
			mapped_user_stack_region = MM.map_object(stack_object);
			return Result(SUCCESS);
		};

		if (do_create_stack().is_error())
			PANIC("NEW_THREAD_STACK_ALLOC_FAIL", "Was unable to allocate virtual memory for a new thread's stack.");

		user_stack = Stack((void*) mapped_user_stack_region->end(), _stack_region->end());
	} else {
		user_stack = Stack((void*) _kernel_stack_region->end());
	}

	//Setup registers
	registers.eflags = 0x202;
	registers.cs = _process->_kernel_mode ? 0x8 : 0x1B;
	registers.eip = (size_t) entry_func;
	registers.eax = 0;
	registers.ebx = 0;
	registers.ecx = 0;
	registers.edx = 0;
	registers.ebp = user_stack.real_stackptr();
	registers.edi = 0;
	registers.esi = 0;
	if(_process->_kernel_mode) {
		registers.ds = 0x10; // ds
		registers.es = 0x10; // es
		registers.fs = 0x10; // fs
		registers.gs = 0x10; // gs
	} else {
		registers.ds = 0x23; // ds
		registers.es = 0x23; // es
		registers.fs = 0x23; // fs
		registers.gs = 0x23; // gs
	}

	//Set up the user stack for the thread arguments
	user_stack.push_sizet((size_t) arg);
	user_stack.push_sizet((size_t) thread_func);
	user_stack.push_sizet(0);

	//Setup the kernel stack with register states
	if(is_kernel_mode())
		setup_kernel_stack(kernel_stack, kernel_stack.real_stackptr(), registers);
	else
		setup_kernel_stack(kernel_stack, user_stack.real_stackptr(), registers);
}

Thread::~Thread() = default;

Process* Thread::process() {
	return _process;
}

pid_t Thread::tid() {
	return _tid;
}

void* Thread::kernel_stack_top() {
	return (void*)(_kernel_stack_region->start() + _kernel_stack_region->size());
}

Thread::State Thread::state() {
	if(process()->state() != Process::ALIVE)
		return (State) process()->state();
	return _state;
}

const char* Thread::state_name() {
	switch(state()) {
		case ALIVE:
			return "Alive";
		case ZOMBIE:
			return "Zombie";
		case DEAD:
			return "Dead";
		case BLOCKED:
			return "Blocked";
		default:
			return "Unknown";
	}
}

bool Thread::is_kernel_mode() {
	return _process->is_kernel_mode();
}

void* Thread::return_value() {
	return _return_value;
}

void Thread::kill() {
	if(is_blocked()) {
		if (_blocker->can_be_interrupted())
			_blocker->interrupt();
		else
			KLog::warn("Thread", "Could not interrupt %s(pid: %d, tid: %d) while killing...", _process->name().c_str(), _process->pid(), _tid);
	}

	if(_in_critical)
		_waiting_to_die = true;
	else
		reap();
}

bool Thread::waiting_to_die() {
	return _waiting_to_die;
}

bool Thread::can_be_run() {
	return state() == ALIVE && !_waiting_to_die;
}

void Thread::enter_critical() {
	_in_critical++;
}

void Thread::leave_critical() {
	_in_critical--;
	if(_waiting_to_die && !_in_critical) {
		_waiting_to_die = false;
		reap();
	}
}

void Thread::block(Blocker& blocker) {
	if(state() != ALIVE || _waiting_to_die)
		PANIC("INVALID_BLOCK", "Tried to block thread %d of PID %d in state %s", _tid, _process->pid(), state_name());
	ASSERT(!_blocker);

	// Check for deadlock
	// TODO: This will only detect if 2 threads are directly deadlocking each other. This will not detect deadlocks involving more than 2 threads.
	if(blocker.responsible_thread()) {
		Thread* responsible = blocker.responsible_thread();
		if(responsible->is_blocked() && !responsible->should_unblock() && responsible->_blocker->responsible_thread() == this) {
			PANIC(
				"THREAD_DEADLOCK",
				"Two threads (%s[%d.%d] and %s[%d.%d]) entered a deadlock.",
				_process->name().c_str(), _process->pid(), _tid,
				responsible->_process->name().c_str(), responsible->_process->pid(), responsible->_tid
			);
		}
	}

	TaskManager::enabled() = false;
	_state = BLOCKED;
	_blocker = &blocker;
	TaskManager::enabled() = true;
	ASSERT(TaskManager::yield());
}

void Thread::unblock() {
	if(!_blocker)
		return;
	_blocker = nullptr;
	if(_state == BLOCKED)
		_state = ALIVE;
}

bool Thread::is_blocked() {
	return _state == BLOCKED;
}

bool Thread::should_unblock() {
	return _blocker && (_blocker->is_ready() || _blocker->was_interrupted());
}

Result Thread::join(const kstd::Arc<Thread>& self_ptr, const kstd::Arc<Thread>& other, UserspacePointer<void*> retp) {
	//See if we're trying to join ourself
	if(other.get() == this)
		return Result(-EDEADLK);

	{
		ScopedLocker __locker1(other->_join_lock);
		ScopedLocker __locker2(_join_lock);

		//Check if the other thread has been joined already
		if (other->_joined || other->_state == DEAD)
			return Result(-EINVAL);

		//Check if the other thread joined this thread
		if (other->_joined_thread == self_ptr)
			return Result(-EDEADLK);

		//Join the other thread
		other->_joined = true;
		_joined_thread = other;
	}

	JoinBlocker blocker(self_ptr, other);
	block(blocker);

	{
		//Reap the joined thread and set the return status
		other->reap();
		if(retp)
			retp.set(other->return_value());

		//Unset the joined thread
		LOCK(_join_lock);
		_joined_thread.reset();
	}

	return Result(SUCCESS);
}

bool Thread::call_signal_handler(int signal) {
	if(is_blocked()) {
		if(_blocker->can_be_interrupted()) {
			_blocker->interrupt();
			unblock();
		} else {
			return false;
		}
	}

	if(signal < 1 || signal >= 32)
		return false;
	auto signal_loc = (size_t) _process->signal_actions[signal].action;
	if(!signal_loc || signal_loc >= HIGHER_HALF)
		return false;

	//Allocate a userspace stack
	auto alloc_user_stack = [&]() -> Result {
		if(!_sighandler_ustack_region) {
			auto user_stack = TRY(AnonymousVMObject::alloc(THREAD_STACK_SIZE));
			_sighandler_ustack_region = TRY(_process->_vm_space->map_object(user_stack, VMProt::RW));
		}
		return Result(SUCCESS);
	};

	if(alloc_user_stack().is_error()) {
		KLog::crit("Thread", "Could not allocate userspace stack for signal handler!");
		return false;
	}

	// Map the user stack into kernel space
	auto k_ustack = MM.map_object(_sighandler_ustack_region->object());

	//Allocate a kernel stack
	if(!_sighandler_kstack_region)
		_sighandler_kstack_region = MM.alloc_kernel_region(THREAD_KERNEL_STACK_SIZE);

	Stack user_stack((void*) k_ustack->end(), _sighandler_ustack_region->end());
	_signal_stack_top = _sighandler_kstack_region->end();

	//Push signal number and fake return address to the stack
	user_stack.push_int(signal);
	user_stack.push_sizet(SIGNAL_RETURN_FAKE_ADDR);

	//Setup signal registers
	signal_registers.eflags = 0x202;
	signal_registers.cs = _process->_kernel_mode ? 0x8 : 0x1B;
	signal_registers.eip = signal_loc;
	signal_registers.eax = 0;
	signal_registers.ebx = 0;
	signal_registers.ecx = 0;
	signal_registers.edx = 0;
	signal_registers.ebp = user_stack.real_stackptr();
	signal_registers.edi = 0;
	signal_registers.esi = 0;
	if(_process->_kernel_mode) {
		signal_registers.ds = 0x10; // ds
		signal_registers.es = 0x10; // es
		signal_registers.fs = 0x10; // fs
		signal_registers.gs = 0x10; // gs
	} else {
		signal_registers.ds = 0x23; // ds
		signal_registers.es = 0x23; // es
		signal_registers.fs = 0x23; // fs
		signal_registers.gs = 0x23; // gs
	}

	//Set up the stack
	setup_kernel_stack(user_stack, user_stack.real_stackptr(), signal_registers);

	//Set the esp register using the real user stack location
	signal_registers.esp = user_stack.real_stackptr();

	//Queue this thread
	_ready_to_handle_signal = true;
	TaskManager::queue_thread(_process->_threads[_tid - 1]);

	return true;
}

bool& Thread::in_signal_handler() {
	return _in_signal;
}

bool Thread::in_critical() {
	return _in_critical;
}

bool& Thread::just_finished_signal() {
	return _just_finished_signal;
}

bool& Thread::ready_to_handle_signal() {
	return _ready_to_handle_signal;
}

void* Thread::signal_stack_top() {
	return (void*) _signal_stack_top;
}

void Thread::handle_pagefault(Registers* regs) {
	size_t err_pos;
	asm volatile ("mov %%cr2, %0" : "=r" (err_pos));

	//If the fault is at the fake signal return address, exit the signal handler
	if(_in_signal && err_pos == SIGNAL_RETURN_FAKE_ADDR) {
		_just_finished_signal = true;
		ASSERT(TaskManager::yield());
	}


	//Otherwise, try CoW and kill the process if it doesn't work
	if(_process->_vm_space->try_pagefault(err_pos).is_error()) {
		if(regs->eip > HIGHER_HALF) {
			PANIC("SYSCALL_PAGEFAULT", "A page fault occurred in the kernel (pid: %d, tid: %d, ptr: 0x%x).", _process->pid(), _tid, err_pos);
		}
		KLog::warn("Thread", "PID %d thread %d made illegal memory access at 0x%x (eip: 0x%x)", _process->pid(), _tid, err_pos, regs->eip);
#ifdef DEBUG
		printf("Userspace stack trace:\n");
		KernelMapper::print_userspace_stacktrace();
#endif
		_process->kill(SIGSEGV);
	}
}

void Thread::setup_kernel_stack(Stack& kernel_stack, size_t user_stack_ptr, Registers& regs) {
	//If usermode, push ss and useresp
	if(!is_kernel_mode()) {
		kernel_stack.push32(0x23);
		kernel_stack.push_sizet(user_stack_ptr);
	}

	//Push EFLAGS, CS, and EIP for iret
	kernel_stack.push32(regs.eflags); // eflags
	kernel_stack.push32(regs.cs); // cs
	kernel_stack.push32(regs.eip); // eip

	kernel_stack.push32(regs.eax);
	kernel_stack.push32(regs.ebx);
	kernel_stack.push32(regs.ecx);
	kernel_stack.push32(regs.edx);
	kernel_stack.push32(regs.ebp);
	kernel_stack.push32(regs.edi);
	kernel_stack.push32(regs.esi);
	kernel_stack.push32(regs.ds);
	kernel_stack.push32(regs.es);
	kernel_stack.push32(regs.fs);
	kernel_stack.push32(regs.gs);

	if(_process->pid() != 0) {
		kernel_stack.push_sizet((size_t) TaskManager::proc_first_preempt);
		kernel_stack.push32(0); //Fake popped EBP
	}

	regs.esp = (size_t) kernel_stack.stackptr();
	regs.useresp = (size_t) user_stack_ptr;
}

void Thread::exit(void* return_value) {
	_return_value = return_value;
	_state = ZOMBIE;
}

void Thread::reap() {
	ASSERT(_state != DEAD);
	_state = DEAD;
	_process->alert_thread_died();
	if(TaskManager::current_thread().get() == this)
		TaskManager::yield();
}