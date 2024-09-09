/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "../tasking/Process.h"
#include "../api/ptrace_internal.h"
#include "../memory/SafePointer.h"
#include "../api/registers.h"

int Process::sys_ptrace(UserspacePointer<struct ptrace_args> args_ptr) {
	if (_user.uid != 0)
		return -EACCES;

	auto args = args_ptr.get();

	kstd::Arc<Tracer> tracer;
	if (args.request == PTRACE_ATTACH) {
		// Find tracee
		auto thread_res = TaskManager::thread_for_tid(args.tid);
		if (thread_res.is_error())
			return -thread_res.code();
		auto tracee = thread_res.value();
		if (tracee->process()->is_kernel_mode())
			return -EACCES;

		tracer = kstd::make_shared<Tracer>(TaskManager::current_process(), tracee);

		auto res = tracee->trace_attach(tracer);
		if (res.is_success()) {
			LOCK(_tracing_lock);
			_tracers.push_back(tracer);
		}
		return -res.code();
	} else {
		LOCK(_tracing_lock);
		for (auto& trac : _tracers) {
			if (trac->tracee_thread()->tid() != args.tid)
				continue;
			tracer = trac;
		}
	}

	if (!tracer)
		return -ENOENT;

	if (tracer->tracee_thread()->state() == Thread::ALIVE)
		return -EBUSY;

	switch (args.request) {
	case PTRACE_GETREGS: {
#if defined(__i386__)
		auto frame = tracer->tracee_thread()->cur_trap_frame();
		if (!frame)
			return -EFAULT;
		UserspacePointer out {(PTraceRegisters*) args.data};
		switch (frame->type) {
		case TrapFrame::IRQ:
			out.set({
				.gp = frame->irq_regs->registers,
				.segment = frame->irq_regs->segment_registers,
				.frame = frame->irq_regs->interrupt_frame
			});
			break;
		case TrapFrame::Syscall:
			out.set({
				.gp = frame->syscall_regs->gp,
				.segment = frame->syscall_regs->seg,
				.frame = frame->syscall_regs->iret
			});
			break;
		case TrapFrame::Fault:
			out.set({
				.gp = frame->fault_regs->registers,
				.segment = frame->fault_regs->segment_registers,
				.frame = frame->fault_regs->interrupt_frame
			});
			break;
		}
		return SUCCESS;
#elif defined(__aarch64__)
		return ENOTSUP; // TODO: aarch64
#endif
	}

	case PTRACE_CONT:
		tracer->tracee_thread()->process()->kill(SIGCONT);
		return SUCCESS;

	case PTRACE_DETACH:
		tracer->tracee_thread()->trace_detach();
		_tracing_lock.acquire();
		for (size_t i = 0; i < _tracers.size(); i++) {
			if (_tracers[i] != tracer)
				continue;
			_tracers.erase(i);
			break;
		}
		_tracing_lock.release();
		return SUCCESS;

	case PTRACE_PEEK:
	case PTRACE_POKE: {
		if ((VirtualAddress) args.addr >= HIGHER_HALF || (VirtualAddress) args.addr % sizeof(VirtualAddress) != 0)
			return -EINVAL;
		auto space = tracer->tracee_thread()->process()->vm_space();
		auto reg_res = space->get_region_containing((VirtualAddress) args.addr);
		if (reg_res.is_error())
			return -EFAULT;
		auto reg = reg_res.value();
		auto obj_offset = (VirtualAddress) args.addr - reg->start() + reg->object_start();
		ASSERT(obj_offset < reg->object()->size());
		auto kreg = MM.map_object(reg->object());
		auto* kptr = (uintptr_t*) (kreg->start() + obj_offset);
		if (args.request == PTRACE_PEEK)
			UserspacePointer((uintptr_t*) args.data).set(*kptr);
		else
			*kptr = (uintptr_t) args.data;
		return SUCCESS;
	}

	default:
		return -EINVAL;
	}
}