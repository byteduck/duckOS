/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include "tasking/Thread.h"

namespace StackWalker {
	struct Frame {
		// TODO: Arch-dependent
		Frame* next_frame;
		VirtualAddress ret_addr;
	};

	Frame* walk_stack(const kstd::Arc<Thread>& thread, uintptr_t* addr_buf, size_t ptr_bufsz, StackWalker::Frame* start_frame);
};
