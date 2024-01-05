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
#include <kernel/kstd/Arc.h>
#include <kernel/memory/Stack.h>
#include <kernel/Result.hpp>
#include "kernel/memory/VMRegion.h"
#include "SpinLock.h"
#include "../memory/PageDirectory.h"
#include "../kstd/queue.hpp"
#include "kernel/kstd/circular_queue.hpp"
#include <kernel/arch/i386/registers.h>

#define THREAD_STACK_SIZE 1048576 //1024KiB
#define THREAD_KERNEL_STACK_SIZE 524288 //512KiB

class Process;
class Blocker;
class ProcessArgs;
template<typename T> class UserspacePointer;
class Thread: public kstd::ArcSelf<Thread> {
public:
	enum State {
		ALIVE = 0,
		ZOMBIE = 1,
		DEAD = 2,
		BLOCKED = 3,
		STOPPED = 4
	};

	Thread(Process* process, tid_t tid, size_t entry_point, ProcessArgs* args);
	Thread(Process* process, tid_t tid, ThreadRegisters& regs);
	Thread(Process* process, tid_t tid, void* (*entry_func)(void* (*)(void*), void*), void* (*thread_func)(void*), void* arg);
	~Thread();

	//Properties
	Process* process();
	tid_t tid();
	void* kernel_stack_top();
	State state();
	const char* state_name();
	bool is_kernel_mode();
	void* return_value();
	void die();
	bool waiting_to_die();
	bool can_be_run();

	//Memory
	[[nodiscard]] PageDirectory* page_directory() const;

	//Critical
	void enter_critical();
	void leave_critical();
	bool in_critical();
	void enter_syscall();
	void leave_syscall();
	bool in_syscall();

	//Blocking and Joining
	void block(Blocker& blocker);
	void unblock();
	bool is_blocked();
	bool should_unblock();
	Result join(const kstd::Arc<Thread>& self_ptr, const kstd::Arc<Thread>& other, UserspacePointer<void*> retp);
	void acquired_lock(SpinLock* lock);
	void released_lock(SpinLock* lock);

	//Signals
	bool& in_signal_handler();
	bool& ready_to_handle_signal();
	bool& just_finished_signal();
	void* signal_stack_top();
	void handle_pending_signal();

	//Misc
	void handle_pagefault(PageFault fault);

	//Thread queue
	void enqueue_thread(Thread* thread);
	Thread* next_thread();

	uint8_t fpu_state[512] __attribute__((aligned(16)));
	ThreadRegisters registers = {};
	ThreadRegisters signal_registers = {};

private:
	friend class Process;
	friend class Reaper;

	void setup_kernel_stack(Stack& kernel_stack, size_t user_stack_ptr, ThreadRegisters& regs);
	void exit(void* return_value);
	void reap();

	bool handle_signal(int signal);
	bool call_signal_handler(int sig);
	void die_from_signal(int signal);
	void dispatch_signal(int signal);

	//Thread stuff
	Process* _process;
	tid_t _tid;
	State _state = ALIVE;
	State _before_stop_state;
	void* _return_value = nullptr;
	int _in_critical = 0; // _in_critical starts as 1 since we leave critical after the first preemption
	bool _waiting_to_die = false;
	bool _in_syscall = false;

	//Memory
	kstd::Arc<VMSpace> m_vm_space;
	kstd::Arc<PageDirectory> m_page_directory;

	//Stack
	kstd::Arc<VMRegion> _kernel_stack_region;
	kstd::Arc<VMRegion> _stack_region;

	//Blocking and Joining
	Blocker* _blocker = nullptr;
	bool _joined = false;
	SpinLock _join_lock;
	kstd::Arc<Thread> _joined_thread;
	kstd::circular_queue<SpinLock*> _held_locks { 100 };

	//Signals
	bool _in_signal = false;
	bool _ready_to_handle_signal = false;
	bool _just_finished_signal = false;
	size_t _signal_stack_top = 0;
	kstd::Arc<VMRegion> _sighandler_ustack_region;
	kstd::Arc<VMRegion> _sighandler_kstack_region;
	uint32_t _pending_signals = 0x0;

	// Thread queue
	Thread* m_next = nullptr;
	Thread* m_prev = nullptr;
};

