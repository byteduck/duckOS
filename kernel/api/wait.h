/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include "types.h"

__DECL_BEGIN

#define WNOHANG    0x1
#define WUNTRACED  0x2

#define __WIFEXITED 0x10
#define WIFEXITED(w) ((w) & __WIFEXITED)
#define WEXITSTATUS(w) ((w) & 0x0f)

#define __WIFSIGNALED 0x20
#define WIFSIGNALED(w) ((w) & __WIFSIGNALED)
#define WTERMSIG(w) ((w) & 0x0f)

#define __WIFSTOPPED 0x40
#define WIFSTOPPED(w) ((w) & __WIFSTOPPED)
#define WSTOPSIG(w) ((w) & 0x0f)

__DECL_END