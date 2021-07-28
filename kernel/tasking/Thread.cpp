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
#include <kernel/memory/Memory.h>
#include <kernel/memory/PageDirectory.h>

Thread::Thread(Process* process, pid_t tid, size_t entry_point, ProcessArgs* args, bool kernel): _tid(tid), ring(kernel ? 0 : 3), _process(process), _kernel_stack_size(THREAD_KERNEL_STACK_SIZE) {
	//Create the kernel stack
	_kernel_stack_base = (void*) PageDirectory::k_alloc_region(_kernel_stack_size).virt->start;

	size_t stack_base;

	if(!kernel) {
		auto stack_reg = _process->page_directory->allocate_stack_region(THREAD_STACK_SIZE, true);
		if (!stack_reg.virt)
			PANIC("NEW_THREAD_STACK_ALLOC_FAIL", "Was unable to allocate virtual memory for a new thread's stack.",
				  true);

		stack_base = stack_reg.virt->start;
		_stack_size = THREAD_STACK_SIZE;
	} else {
		stack_base = (size_t) _kernel_stack_base;
		_stack_size = THREAD_KERNEL_STACK_SIZE;
	}

	//Setup registers
	registers.eflags = 0x202;
	registers.cs = kernel ? 0x8 : 0x1B;
	registers.eip = entry_point;
	registers.eax = 0;
	registers.ebx = 0;
	registers.ecx = 0;
	registers.edx = 0;
	registers.ebp = stack_base;
	registers.edi = 0;
	registers.esi = 0;
	if(kernel) {
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

	//Setup stack
	if(!kernel) {
		//Set up the user stack for the program arguments
		auto user_stack = (uint32_t *) (stack_base + _stack_size);
		user_stack = (uint32_t *) args->setup_stack(user_stack);
		*--user_stack = 0; //Honestly? Not sure why this is needed but nothing works without it :)

		//Setup the kernel stack with register states
		auto *kernel_stack = (uint32_t *) ((size_t) _kernel_stack_base + _kernel_stack_size);
		setup_stack(kernel_stack, user_stack, registers);
	} else {
		auto *stack = (uint32_t*) (stack_base + _stack_size);
		stack = (uint32_t *) args->setup_stack(stack);
		*--stack = 0;
		setup_stack(stack, stack, registers);
	}
}

Thread::Thread(Process* process, pid_t tid, Registers& regs): _process(process), _tid(tid), _kernel_stack_size(THREAD_KERNEL_STACK_SIZE), registers(regs), ring(3) {
	//Allocate kernel stack
	_kernel_stack_base = (void*) PageDirectory::k_alloc_region(_kernel_stack_size).virt->start;

	//Setup registers and stack
	registers.eax = 0; // fork() in child returns zero
	auto* user_stack = (size_t*) regs.useresp;
	auto* kernel_stack = (size_t*) ((size_t) _kernel_stack_base + _kernel_stack_size);
	setup_stack(kernel_stack, user_stack, registers);
}

Thread::~Thread() = default;

kstd::shared_ptr<Process>& Thread::process() {
	return _process;
}

pid_t Thread::tid() {
	return _tid;
}

void* Thread::kernel_stack_top() {
	return (void*)((size_t)_kernel_stack_base + _kernel_stack_size);
}

void Thread::block(Blocker& blocker) {
	ASSERT(state == ALIVE);
	ASSERT(!_blocker);
	TaskManager::enabled() = false;
	state = BLOCKED;
	_blocker = &blocker;
	TaskManager::enabled() = true;
	ASSERT(TaskManager::yield());
}

void Thread::unblock() {
	if(!_blocker)
		return;
	_blocker = nullptr;
	if(state == BLOCKED)
		state = ALIVE;
}

bool Thread::is_blocked() {
	return state == BLOCKED;
}

bool Thread::should_unblock() {
	return _blocker && _blocker->is_ready();
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
	_sighandler_ustack_region = _process->page_directory->allocate_region(THREAD_STACK_SIZE, true);
	if(!_sighandler_ustack_region.virt) {
		printf("FATAL: Failed to allocate sighandler user stack for pid %d!\n", _process->pid());
		_process->kill(SIGKILL);
		return false;
	}

	//Allocate a kernel stack
	_sighandler_kstack_region = PageDirectory::k_alloc_region(THREAD_KERNEL_STACK_SIZE);
	if(!_sighandler_kstack_region.virt) {
		printf("FATAL: Failed to allocate sighandler kernel stack for pid %d!\n", _process->pid());
		_process->kill(SIGKILL);
		return false;
	}

	//Map the user stack into the kernel temporarily
	auto k_ustack_region = PageDirectory::k_map_physical_region(_sighandler_ustack_region.phys, true);

	auto* user_stack = (size_t*) (k_ustack_region.virt->start + THREAD_STACK_SIZE);
	_signal_stack_top = _sighandler_kstack_region.virt->start + _sighandler_kstack_region.virt->size;

	//Push signal number and fake return address to the stack
	*--user_stack = signal;
	*--user_stack = SIGNAL_RETURN_FAKE_ADDR;

	//Calculate the current location in the user stack in program space
	size_t real_userstack = _sighandler_ustack_region.virt->start + THREAD_STACK_SIZE - sizeof(size_t) * 2;

	//Setup signal registers
	signal_registers.eflags = 0x202;
	signal_registers.cs = ring == 0 ? 0x8 : 0x1B;
	signal_registers.eip = signal_loc;
	signal_registers.eax = 0;
	signal_registers.ebx = 0;
	signal_registers.ecx = 0;
	signal_registers.edx = 0;
	signal_registers.ebp = real_userstack;
	signal_registers.edi = 0;
	signal_registers.esi = 0;
	if(ring == 0) {
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

	//Take note of the stack location before
	auto* stack_before = user_stack;

	//Set up the stack
	setup_stack(user_stack, (uint32_t*) real_userstack, signal_registers);

	//Set the esp register using the real user stack location
	signal_registers.esp = real_userstack - ((size_t) stack_before - (size_t) user_stack);

	//Unmap the user stack from kernel space
	PageDirectory::k_free_virtual_region(k_ustack_region);

	_ready_to_handle_signal = true;
	return true;
}

bool& Thread::in_signal_handler() {
	return _in_signal;
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
		PageDirectory::k_free_region(_sighandler_kstack_region);
		_process->page_directory->free_region(_sighandler_ustack_region);
		ASSERT(TaskManager::yield());
	}

	//Otherwise, try CoW and kill the process if it doesn't work
	if(!_process->page_directory->try_cow(err_pos))
		_process->kill(SIGSEGV);
}

void Thread::setup_stack(uint32_t*& kernel_stack, const uint32_t* userstack, Registers& regs) {
	//If usermode, push ss and useresp
	if(ring != 0) {
		*--kernel_stack = 0x23;
		*--kernel_stack = (size_t) userstack;
	}

	//Push EFLAGS, CS, and EIP for iret
	*--kernel_stack = regs.eflags; // eflags
	*--kernel_stack = regs.cs; // cs
	*--kernel_stack = regs.eip; // eip

	*--kernel_stack = regs.eax;
	*--kernel_stack = regs.ebx;
	*--kernel_stack = regs.ecx;
	*--kernel_stack = regs.edx;
	*--kernel_stack = regs.ebp;
	*--kernel_stack = regs.edi;
	*--kernel_stack = regs.esi;
	*--kernel_stack = regs.ds;
	*--kernel_stack = regs.es;
	*--kernel_stack = regs.fs;
	*--kernel_stack = regs.gs;

	if(_process->pid() != 0) {
		*--kernel_stack = (size_t) TaskManager::proc_first_preempt;
		*--kernel_stack = 0; //Fake popped EBP
	}

	regs.esp = (size_t) kernel_stack;
	regs.useresp = (size_t) userstack;
}

void Thread::die() {
	if(is_blocked())
		unblock();
	state = DEAD;
}