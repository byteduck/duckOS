/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#if defined(__i386__)
#include "i386/Processor.h"
#elif defined(__aarch64__)
#include "aarch64/Processor.h"
#endif