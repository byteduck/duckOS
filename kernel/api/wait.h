/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include "types.h"

__DECL_BEGIN

#define WNOHANG    0x1
#define WUNTRACED  0x2

#define __WIFEXITED 0x1000
#define WIFEXITED(w) ((w) & __WIFEXITED)
#define WEXITSTATUS(w) ((w) & 0x00ff)

#define __WIFSIGNALED 0x2000
#define WIFSIGNALED(w) ((w) & __WIFSIGNALED)
#define WTERMSIG(w) ((w) & 0x00ff)

#define __WIFSTOPPED 0x4000
#define WIFSTOPPED(w) ((w) & __WIFSTOPPED)
#define WSTOPSIG(w) ((w) & 0x00ff)

__DECL_END