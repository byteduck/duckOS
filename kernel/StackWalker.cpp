/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "StackWalker.h"
#include "tasking/TaskManager.h"
#include "memory/MemoryManager.h"

StackWalker::Frame* StackWalker::walk_stack(const kstd::Arc<Thread>& thread, uintptr_t* addr_buf, size_t addr_bufsz, StackWalker::Frame* start_frame) {
#if defined(__i386__)
	Frame* cur_frame;
	if (start_frame)
		cur_frame = start_frame;
	else if (thread == TaskManager::current_thread())
		cur_frame = (Frame*) __builtin_frame_address(0);
	else
		cur_frame = ((Frame*) (thread->in_signal_handler() ? thread->signal_registers.gp.esp : thread->registers.gp.esp))->next_frame;

	size_t count = 0;

	auto do_frame = [&] (Frame* frame) {
		cur_frame = frame->next_frame;
		*(addr_buf++) = frame->ret_addr;
		addr_bufsz--;
		count++;
	};

	while (cur_frame && addr_bufsz) {
		if ((uintptr_t) cur_frame >= HIGHER_HALF) {
			ASSERT(MM.kernel_page_directory.is_mapped((VirtualAddress) cur_frame, false));
			do_frame(cur_frame);
		} else {
			auto frame_paddr = thread->page_directory()->get_physaddr(cur_frame);
			auto frame_page = frame_paddr / PAGE_SIZE;
			// TODO: Could be more efficient about quickmapping.
			MM.with_quickmapped(frame_page, [&] (void* pagemem) {
				do_frame((Frame*) ((VirtualAddress) pagemem + (frame_paddr % PAGE_SIZE)));
			});
		}
	}

	return cur_frame;
#elif defined(__aarch64__)
	// TODO: aarch64
	return nullptr;
#endif
}
