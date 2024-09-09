/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once


#define SCTLR_RESERVED ((3 << 28) | (3 << 22) | (1 << 20) | (1 << 11))
#define SCTLR_MMU_ENABLE (1 << 0)
#define SCTLR_CACHE_ENABLE (0x1 << 2)
#define SCTLR_ICACHE_ENABLE (0x1 << 12)

#ifdef __cplusplus
#include <kernel/kstd/types.h>
namespace Aarch64::Regs {
	struct ID_AA64MMFR0_EL1 {
		uint64_t pa_range : 4;
		uint64_t asid_bits : 4;
		uint64_t big_endian : 4;
		uint64_t sns_mem : 4;
		uint64_t big_end_el0 : 4;
		uint64_t tgran_16 : 4;
		uint64_t tgran_64 : 4;
		uint64_t tgran_4 : 4;
		uint64_t tgran_16_2 : 4;
		uint64_t tgran_64_2 : 4;
		uint64_t tgran_4_2 : 4;
		uint64_t exs : 4;
		uint64_t : 8;
		uint64_t fgt : 4;
		uint64_t ecv : 4;
	};

	struct SCTLR_EL1 {
		bool m: 1;
		bool a: 1;
		bool c: 1;
		bool sa: 1;
		bool sa0: 1;
		bool cp15ben: 1;
		bool naa: 1;
		bool itd: 1;
		bool sed: 1;
		bool uma: 1;
		bool enrctx: 1;
		bool eos: 1;
		bool i: 1;
		bool endb: 1;
		bool dze: 1;
		bool uct: 1;
		bool ntwi: 1;
		bool _reserved1: 1 = false;
		bool ntwe: 1;
		bool wxn: 1;
		bool tscxt: 1;
		bool iesb: 1;
		bool eis: 1;
		bool span: 1;
		bool e0e: 1;
		bool ee: 1;
		bool uci: 1;
		bool enda: 1;
		bool ntlsmd: 1;
		bool lsmaoe: 1;
		bool enib: 1;
		bool enia: 1;
		uint64_t _reserved2: 3 = 0;
		bool bt0: 1;
		bool bt1: 1;
		bool itfsb: 1;
		uint64_t tcf0: 2;
		uint64_t tcf: 2;
		bool ata0: 1;
		bool ata: 1;
		bool dssbs: 1;
		bool tweden: 1;
		uint64_t twedel: 4;
		uint64_t _reserved3: 4 = 0;
		bool enasr: 1;
		bool enas0: 1;
		bool enals: 1;
		bool epan: 1;
		uint64_t _reserved4: 6 = 0;

		static constexpr SCTLR_EL1 default_val() {
			SCTLR_EL1 ret;
			ret.sa = true;
			ret.sa0 = true;
			ret.itd = true;
			ret.sed = true;
			ret.eos = true;
			ret.tscxt = true;
			ret.iesb = true;
			ret.eis = true;
			ret.span = true;
			ret.lsmaoe = true;
			ret.ntlsmd = true;
			return ret;
		}
	};

	union GPRegisters {
		uint64_t regs[31];
		struct {
			uint64_t x0;
			uint64_t x1;
			uint64_t x2;
			uint64_t x3;
			uint64_t x4;
			uint64_t x5;
			uint64_t x6;
			uint64_t x7;
			uint64_t x8;
			uint64_t x9;
			uint64_t x10;
			uint64_t x11;
			uint64_t x12;
			uint64_t x13;
			uint64_t x14;
			uint64_t x15;
			uint64_t x16;
			uint64_t x17;
			uint64_t x18;
			uint64_t x19;
			uint64_t x20;
			uint64_t x21;
			uint64_t x22;
			uint64_t x23;
			uint64_t x24;
			uint64_t x25;
			uint64_t x26;
			uint64_t x27;
			uint64_t x28;
			uint64_t x29;
			uint64_t x30;
		};
	};
}

struct ISRRegisters {};
struct ThreadRegisters {
	Aarch64::Regs::GPRegisters gp;
	uint64_t sp;
	uint64_t spsr_el1;
	uint64_t elr_el1;
	uint64_t tpidr_el0;
	uint64_t ttbr0_el1;
};
struct IRQRegisters {};
struct TrapFrame {
	TrapFrame* prev;
};

#endif