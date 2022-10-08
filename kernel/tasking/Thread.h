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

#pragma once

#include <kernel/kstd/kstddef.h>
#include <kernel/kstd/unix_types.h>
#include <kernel/kstd/shared_ptr.hpp>
#include <kernel/memory/LinkedMemoryRegion.h>
#include <kernel/memory/Stack.h>
#include <kernel/Result.hpp>

#define THREAD_STACK_SIZE 1048576 //1024KiB
#define THREAD_KERNEL_STACK_SIZE 4096 //4KiB

class Process;
class Blocker;
class ProcessArgs;
template<typename T> class UserspacePointer;
class Thread {
public:
	enum State {
		ALIVE = 0,
		ZOMBIE = 1,
		DEAD = 2,
		BLOCKED = 3
	};

	Thread(Process* process, tid_t tid, size_t entry_point, ProcessArgs* args);
	Thread(Process* process, tid_t tid, Registers& regs);
	Thread(Process* process, tid_t tid, void* (*entry_func)(void* (*)(void*), void*), void* (*thread_func)(void*), void* arg);
	~Thread();

	//Properties
	Process* process();
	pid_t tid();
	void* kernel_stack_top();
	State state();
	const char* state_name();
	bool is_kernel_mode();
	void* return_value();
	void kill();
	bool waiting_to_die();
	bool can_be_run();

	//Critical
	void enter_critical();
	void leave_critical();

	//Blocking and Joining
	void block(Blocker& blocker);
	void unblock();
	bool is_blocked();
	bool should_unblock();
	Result join(const kstd::shared_ptr<Thread>& self_ptr, const kstd::shared_ptr<Thread>& other, UserspacePointer<void*> retp);

	//Signals
	bool call_signal_handler(int sig);
	bool& in_signal_handler();
	bool in_critical();
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
	void exit(void* return_value);
	void reap();

	//Thread stuff
	Process* _process;
	tid_t _tid;
	State _state = ALIVE;
	void* _return_value = nullptr;
	int _in_critical = 0;
	bool _waiting_to_die = false;

	//Stack
	LinkedMemoryRegion _kernel_stack_region;
	LinkedMemoryRegion _stack_region;

	//Blocking and Joining
	Blocker* _blocker = nullptr;
	bool _joined = false;
	SpinLock _join_lock;
	kstd::shared_ptr<Thread> _joined_thread;

	//Signals
	bool _in_signal = false;
	bool _ready_to_handle_signal = false;
	bool _just_finished_signal = false;
	size_t _signal_stack_top = 0;
	LinkedMemoryRegion _sighandler_ustack_region;
	LinkedMemoryRegion _sighandler_kstack_region;
};

