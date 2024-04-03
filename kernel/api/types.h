/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include "cdefs.h"
#include "stdint.h"

__DECL_BEGIN

typedef __SIZE_TYPE__ size_t;
typedef int32_t ssize_t;
typedef int pid_t;
typedef int tid_t;
typedef unsigned long ino_t;
typedef short dev_t;
typedef uint32_t mode_t;
typedef unsigned short nlink_t;
typedef unsigned short uid_t;
typedef unsigned short gid_t;
typedef long off_t;
typedef long blksize_t;
typedef long blkcnt_t;
typedef int id_t;

typedef long suseconds_t;
typedef unsigned long useconds_t;

__DECL_END