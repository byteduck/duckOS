/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once
#include "../registers.h"

// HCR_EL2
#define HCR_EL2_VAL (1 << 31)

// SCR
#define SCR_EL3_VAL ((1 << 10) | (3 << 4) | 1)
#define SCR_EL2_VAL ((1 << 10) | (3 << 4) | 1)

// SPSR
#define SPSR_EL3_VAL ((7 << 6) | 5)
#define SPSR_EL2_VAL ((7 << 6) | 5)

#define CPACR_FPEN 0x300000

#ifdef __cplusplus
extern "C" void setup_exception_level();
#endif