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

#include "signal.h"
#include "sys/syscall.h"
#include "unistd.h"

void (*signal(int sig, sighandler_t func))(int) {
	sigaction_t act = {func, 0, 0};
	sigaction_t old_act;
	sigaction(sig, &act, &old_act);
	return old_act.sa_sigaction;
}

int raise(int sig) {
	return kill(getpid(), sig);
}

int kill(pid_t pid, int sig) {
	return syscall3(SYS_KILL, pid, sig);
}

void sigaction(int signum, const struct sigaction* act, const struct sigaction* oldact) {
	syscall4(SYS_SIGACTION, signum, (int) act, (int) oldact);
}

int sigemptyset(sigset_t* set) {
	*set = 0;
	return 0;
}

int sigfillset(sigset_t* set) {
	*set = ~UINT32_C(0);
	return 0;
}

int sigaddset(sigset_t* set, int sig) {
	if (sig < 1 || sig > 32) {
		errno = EINVAL;
		return -1;
	}
	*set |= 1 << (sig - 1);
	return 0;
}

int sigdelset(sigset_t* set, int sig) {
	if (sig < 1 || sig > 32) {
		errno = EINVAL;
		return -1;
	}
	*set &= ~(1 << (sig - 1));
	return 0;
}