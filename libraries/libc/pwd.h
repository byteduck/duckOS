/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2023 Chaziz */

#pragma once

#include "sys/types.h"
#include <kernel/api/time.h>

__DECL_BEGIN

struct passwd {
	char *pw_name;
	char *pw_passwd;
	uid_t pw_uid;
	gid_t pw_gid;
	time_t pw_change;
	char *pw_class;
	char *pw_gecos;
	char *pw_dir;
	char *pw_shell;
	time_t pw_expire;
}; 

__DECL_END