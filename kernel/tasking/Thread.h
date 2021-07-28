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

#ifndef DUCKOS_THREAD_H
#define DUCKOS_THREAD_H

#include <kernel/kstd/kstddef.h>
#include <kernel/kstd/unix_types.h>
#include <kernel/kstd/shared_ptr.hpp>
#include <kernel/memory/LinkedMemoryRegion.h>
#include <kernel/memory/Stack.h>

#define THREAD_STACK_SIZE 1048576 //1024KiB
#define THREAD_KERNEL_STACK_SIZE 4096 //4KiB

class Process;
class Blocker;
class ProcessArgs;
class Thread {
public:
	enum State {
		ALIVE,
		BLOCKED,
		DEAD
	};

	Thread(Process* process, pid_t tid, size_t entry_point, ProcessArgs* args);
	Thread(Process* process, pid_t tid, Registers& regs);
	~Thread();

	//Properties
	kstd::shared_ptr<Process>& process();
	pid_t tid();
	void* kernel_stack_top();
	State state();
	bool is_kernel_mode();

	//Blocking
	void block(Blocker& blocker);
	void unblock();
	bool is_blocked();
	bool should_unblock();

	//Signals
	bool call_signal_handler(int sig);
	bool& in_signal_handler();
	bool& ready_to_handle_signal();
	bool& just_finished_signal();
	void* signal_stack_top();

	//Misc
	void handle_pagefault(Registers* regs);

	uint8_t fpu_state[512] __attribute__((aligned(16)));
	Registers registers = {};
	Registers signal_registers = {};

private:
	friend class Process;

	void setup_kernel_stack(Stack& kernel_stack, size_t user_stack_ptr, Registers& regs);
	void die();

	//Thread stuff
	kstd::shared_ptr<Process> _process;
	pid_t _tid;
	State _state = ALIVE;

	//Stack
	LinkedMemoryRegion _kernel_stack_region;
	LinkedMemoryRegion _stack_region;

	//Blocking
	Blocker* _blocker = nullptr;

	//Signals
	bool _in_signal = false;
	bool _ready_to_handle_signal = false;
	bool _just_finished_signal = false;
	size_t _signal_stack_top = 0;
	LinkedMemoryRegion _sighandler_ustack_region;
	LinkedMemoryRegion _sighandler_kstack_region;
};

#endif //DUCKOS_THREAD_H
