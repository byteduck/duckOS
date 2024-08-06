/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#if defined(__i386__)
#include <kernel/arch/i386/PageDirectory.h>
#elif defined(__aarch64__)
#include <kernel/arch/aarch64/PageDirectory.h>
#endif