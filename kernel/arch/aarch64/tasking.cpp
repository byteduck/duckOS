/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

namespace TaskManager {
	void idle_task() {
		// TODO: aarch64
		while (1);
	}

	extern "C" void preempt_asm() {
		// TODO: aarch64
	}

	extern "C" void proc_first_preempt() {
		// TODO: aarch64
	}
}