/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "../tasking/Process.h"

void Process::sys_exit(int status) {
	_exit_status = status;
	_died_gracefully = true;
	kill(SIGKILL);
}