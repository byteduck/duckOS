/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include "types.h"

__DECL_BEGIN

#define	SIGHUP	1	/* hangup */
#define	SIGINT	2	/* interrupt */
#define	SIGQUIT	3	/* quit */
#define	SIGILL	4	/* illegal instruction (not reset when caught) */
#define	SIGTRAP	5	/* trace trap (not reset when caught) */
#define	SIGABRT 6	/* used by abort, replace SIGIOT in the future */
#define	SIGEMT	7	/* EMT instruction */
#define	SIGFPE	8	/* floating point exception */
#define	SIGKILL	9	/* kill (cannot be caught or ignored) */
#define	SIGBUS	10	/* bus error */
#define	SIGSEGV	11	/* segmentation violation */
#define	SIGSYS	12	/* bad argument to system call */
#define	SIGPIPE	13	/* write on a pipe with no one to read it */
#define	SIGALRM	14	/* alarm clock */
#define	SIGTERM	15	/* software termination signal from kill */
#define	SIGURG	16	/* urgent condition on IO channel */
#define	SIGSTOP	17	/* sendable stop signal not from tty */
#define	SIGTSTP	18	/* stop signal from tty */
#define	SIGCONT	19	/* continue a stopped process */
#define	SIGCHLD	20	/* to parent on child stop or exit */
#define	SIGTTIN	21	/* to readers pgrp upon background tty read */
#define	SIGTTOU	22	/* like TTIN for output if (tp->t_local&LTOSTOP) */
#define	SIGIO	23	/* input/output possible signal */
#define	SIGXCPU	24	/* exceeded CPU time limit */
#define	SIGXFSZ	25	/* exceeded file size limit */
#define	SIGVTALRM 26	/* virtual time alarm */
#define	SIGPROF	27	/* profiling time alarm */
#define	SIGWINCH 28	/* window changed */
#define	SIGLOST 29	/* resource lost (eg, record-lock lost) */
#define	SIGUSR1 30	/* user defined signal 1 */
#define	SIGUSR2 31	/* user defined signal 2 */
#define NSIG	32

#define SIG_DFL ((sighandler_t)0)
#define SIG_ERR ((sighandler_t)1)
#define SIG_IGN ((sighandler_t)2)

#define SIG_BLOCK 0
#define SIG_UNBLOCK 1
#define SIG_SETMASK 2

typedef uint32_t sig_atomic_t;
typedef void (*sighandler_t)(int);
typedef unsigned long sigset_t;

union sigval {
	int sival_int;
	void* sival_ptr;
};

typedef struct siginfo {
	int si_signo;
	int si_code;
	int si_errno;
	pid_t si_pid;
	uid_t si_uid;
	void* si_addr;
	int si_status;
	int si_band;
	union sigval si_value;
} siginfo_t;

typedef struct sigaction {
	union {
		void (*sa_sigaction)(int, siginfo_t*, void *);
		sighandler_t sa_handler;
	};
	sigset_t sa_mask;
	int sa_flags;
} sigaction_t;

__DECL_END