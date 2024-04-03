/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include "types.h"

__DECL_BEGIN

struct sched_param {
	int sched_priority;
};

#define THREAD_PRIORITY_MIN 0
#define THREAD_PRIORITY_LOW 1
#define THREAD_PRIORITY_NORMAL 2
#define THREAD_PRIORITY_HIGH 3
#define THREAD_PRIORITY_MAX 4

#define SCHED_FIFO 0
#define SCHED_RR 1
#define SCHED_OTHER 2
#define SCHED_BATCH 3

__DECL_END