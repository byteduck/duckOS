/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include "types.h"
#include "time.h"

#define RUSAGE_SELF 1
#define RUSAGE_CHILDREN 2
#define RLIMIT_CORE 1
#define RLIMIT_CPU 2
#define RLIMIT_DATA 3
#define RLIMIT_FSIZE 4
#define RLIMIT_NOFILE 5
#define RLIMIT_STACK 6
#define RLIMIT_AS 7
#define RLIMIT_NLIMITS 8
#define RLIM_INFINITY SIZE_MAX
#define PRIO_PROCESS 0
#define PRIO_PGRP 1
#define PRIO_USER 2

__DECL_BEGIN

struct rusage {
	struct timeval ru_utime;
	struct timeval ru_stime;
	long ru_maxrss;
	long ru_ixrss;
	long ru_idrss;
	long ru_isrss;
	long ru_minflt;
	long ru_majflt;
	long ru_nswap;
	long ru_inblock;
	long ru_oublock;
	long ru_msgsnd;
	long ru_msgrcv;
	long ru_nsignals;
	long ru_nvcsw;
	long ru_nivcsw;
};

typedef size_t rlim_t;
struct rlimit {
	rlim_t rlim_cur;
	rlim_t rlim_max;
};

__DECL_END