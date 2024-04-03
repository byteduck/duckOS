/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include <kernel/api/sched.h>

__DECL_BEGIN

int sched_yield();
int sched_get_priority_min(int policy);
int sched_get_priority_max(int policy);
int sched_setparam(pid_t pid, const struct sched_param* param);
int sched_getparam(pid_t pid, struct sched_param* param);

__DECL_END