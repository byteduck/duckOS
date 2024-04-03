/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "resource.h"
#include "../string.h"

int getrusage(int who, struct rusage* usage) {
	// TODO
	memset(usage, 0, sizeof(struct rusage));
	return 0;
}

int getrlimit(int name, struct rlimit* limit) {
	// TODO
	return -1;
}

int setrlimit(int name, const struct rlimit* limit) {
	// TODO
	return -1;
}

int getpriority(int name, id_t id) {
	// TODO
	return -1;
}

int setpriority(int name, id_t id, int prio) {
	// TODO
	return -1;
}