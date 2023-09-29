/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#define EFLAGS_ID (0x1lu << 21)

#define BIT(n) (0x1lu << (n))

enum CPUIDOp: uint32_t {
	Vendor   = 0x0,
	Features = 0x1
};

struct CPUFeatures {
	union {
		struct {
			bool SSE3       : 1;
			bool PCLMULQDQ  : 1;
			bool DTES64     : 1;
			bool MONITOR    : 1;
			bool DSCPL      : 1;
			bool VMX        : 1;
			bool SMX        : 1;
			bool EIST       : 1;
			bool TM2        : 1;
			bool SSSE3      : 1;
			bool CNXTID     : 1;
			bool SDBG       : 1;
			bool FMA        : 1;
			bool CMPXCHG16B : 1;
			bool XTPRUC     : 1;
			bool PDCM       : 1;
			bool            : 1;
			bool PCID       : 1;
			bool DCA        : 1;
			bool SSE4_1     : 1;
			bool SSE4_2     : 1;
			bool X2APIC     : 1;
			bool MOVBE      : 1;
			bool POPCNT     : 1;
			bool TSCDEADLN  : 1;
			bool AESNI      : 1;
			bool XSAVE      : 1;
			bool OSXSAVE    : 1;
			bool AVX        : 1;
			bool F16C       : 1;
			bool RDRAND     : 1;
			bool            : 1;
		};
		uint32_t ecx_value;
	};

	union {
		struct {
			bool FPU        : 1;
			bool VME        : 1;
			bool DE         : 1;
			bool PSE        : 1;
			bool TSC        : 1;
			bool MSR        : 1;
			bool PAE        : 1;
			bool MCE        : 1;
			bool CX8        : 1;
			bool APIC       : 1;
			bool            : 1;
			bool SEP        : 1;
			bool MTRR       : 1;
			bool PGE        : 1;
			bool MCA        : 1;
			bool CMOV       : 1;
			bool PAT        : 1;
			bool PSE36      : 1;
			bool PSN        : 1;
			bool CLFSH      : 1;
			bool            : 1;
			bool DS         : 1;
			bool ACPI       : 1;
			bool MMX        : 1;
			bool FXSR       : 1;
			bool SSE        : 1;
			bool SSE2       : 1;
			bool SS         : 1;
			bool HTT        : 1;
			bool TM         : 1;
			bool            : 1;
			bool PBE        : 1;
		};
		uint32_t edx_value;
	};
};