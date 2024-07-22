/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

namespace TaskManager {
	void idle_task() {
		while(1) {
			asm volatile("hlt");
		}
	}
}