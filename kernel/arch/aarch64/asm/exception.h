/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

// SCTLR_EL1
#define SCTLR_RESERVED ((3 << 28) | (3 << 22) | (1 << 20) | (1 << 11))
#define SCTLR_MMU_ENABLED (1 << 0)

// HCR_EL2
#define HCR_EL2_VAL (1 << 31)

// SCR_EL3
#define SCR_EL3_VAL ((1 << 10) | (3 << 4) | 1)

// SPSR_EL3
#define SPSR_EL3_VAL ((7 << 6) | 5)

#define CPACR_FPEN 0x300000

#ifdef __cplusplus
extern "C" void setup_exception_level();
#endif