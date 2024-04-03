/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include <kernel/api/resource.h>

__DECL_BEGIN

int getrusage(int who, struct rusage* usage);
int getrlimit(int name, struct rlimit* limit);
int setrlimit(int name, const struct rlimit* limit);
int getpriority(int name, id_t id);
int setpriority(int name, id_t id, int prio);

__DECL_END