/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include <kernel/kstd/types.h>

namespace Aarch64::Regs {
	union MAIR {
		enum DeviceMemory: uint8_t {
			nGnRnE = 0b00 << 2, // non-Gathering, non-Reordering, non-Early Write Acknowledgement
			nGnRE  = 0b01 << 2, // non-Gathering, non-Reordering, Early Write Acknowledgement
			nGRE   = 0b10 << 2, // non-Gathering, Reordering, Early Write Acknowledgement
			GRE    = 0b11 << 2  // Gathering, Reordering, Early Write Acknowledgement
		};

		enum NormalMemory: uint8_t {
			NonCacheable = 0b0100,
			Cacheable = 0b1111
		};

		// Bits to shift NormalMemory by
		enum CacheLevel: int {
			Outer = 4,
			Inner = 0
		};

		uint8_t sections[8];
		uint64_t value;
	};

	union TCR {
		struct {
			uint64_t t0sz : 6;
			uint64_t : 1;
			bool epd0 : 1;
			uint64_t irgn0 : 2;
			uint64_t orgn0 : 2;
			uint64_t sh0 : 2;
			uint64_t tg0 : 2;

			uint64_t t1sz : 6;
			bool a1 : 1;
			bool epd1 : 1;
			uint64_t irgn1 : 2;
			uint64_t orgn1 : 2;
			uint64_t sh1 : 2;
			uint64_t tg1 : 2;

			uint64_t ips : 3;
			uint64_t : 1;
			bool as : 1;
			bool tbi0 : 1;
			bool tbi1 : 1;
			bool ha : 1;
			bool hd : 1;
			bool hpd0 : 1;
			bool hpd1 : 1;
			bool hwu059 : 1;
			bool hwu060 : 1;
			bool hwu061 : 1;
			bool hwu062 : 1;
			bool hwu159 : 1;
			bool hwu160 : 1;
			bool hwu161 : 1;
			bool hwu162 : 1;
			bool tbid0 : 1;
			bool tbid1 : 1;
			bool nfd0 : 1;
			bool nfd1 : 1;
			bool e0pd0 : 1;
			bool e0pd1 : 1;
			bool tcma0 : 1;
			bool tcma1 : 1;
			bool ds : 1;
			bool mtx0 : 1;
			bool mxt1 : 1;
			uint64_t : 2;
		};
		uint64_t value;
	};
}

namespace Aarch64::MMU {
	constexpr size_t pte_size = 4096;
	constexpr size_t pmd_size = pte_size * 512;
	constexpr size_t pud_size = pmd_size * 512;
	constexpr size_t pgd_size = pud_size * 512;

	constexpr uint64_t descriptor_addr(size_t addr) {
		return ((addr) >> 12) & 0xFFFFFFFFF;
	}

	constexpr uint64_t get_descriptor_addr(size_t addr) {
		return addr << 12;
	}

	constexpr uint64_t pte_index(size_t addr) {
		return ((addr) >> 12) & 0x1FF;
	}

	constexpr uint64_t pte_page_index(size_t page) {
		return page & 0x1FF;
	}

	constexpr uint64_t pmd_index(size_t addr) {
		return ((addr) >> 21) & 0x1FF;
	}

	constexpr uint64_t pmd_page_index(size_t page) {
		return ((page) >> 9) & 0x1FF;
	}

	constexpr uint64_t pud_index(size_t addr) {
		return ((addr) >> 30) & 0x1FF;
	}

	constexpr uint64_t pud_page_index(size_t page) {
		return ((page) >> 18) & 0x1FF;
	}

	constexpr uint64_t pgd_index(size_t addr) {
		return ((addr) >> 39) & 0x1FF;
	}

	constexpr uint64_t pgd_page_index(size_t page) {
		return ((page) >> 27) & 0x1FF;
	}

	enum Attr: uint8_t {
		NormalUncacheable = 0,
		NormalCacheable   = 1,
		Device_nGnRnE     = 2,
		Device_nGnRE      = 3
	};

	struct PageDescriptor {
		bool valid: 1;

		enum Type: bool {
			Block = false,
			Table = true
		} type: 1;

		uint64_t attr_index : 3;

		enum Security: bool {
			Secure = false,
			Nonsecure = true
		} security : 1;

		enum Perms: uint64_t {
			pRW    = 0b00,
			pRWuRW = 0b01,
			pR     = 0b10,
			pRuR   = 0b11
		} read_write : 2;

		enum Shareability: uint64_t {
			NonShareable   = 0b00,
			Reserved       = 0b01,
			OuterShareable = 0b10,
			InnerShareable = 0b11
		} shareability : 2;

		bool access : 1;
		bool _zero : 1 = false;
		uint64_t address : 36;
		uint64_t _reserved : 16 = 0;
	};

	struct TableDescriptor {
		bool valid : 1;
		enum Type: bool {
			Block = false,
			Table = true
		} type : 1 = Table;
		uint64_t _res0 : 10 = 0;
		uint64_t address : 36;
		uint64_t _res1 : 11 = 0;
		uint64_t heirarchical_perms : 4 = 0;
		enum Security: bool {
			Secure = false,
			Nonsecure = true
		} security : 1;
	};

	void mmu_init();

	/**
	 * Allocates memory from early page table memory.
	 */
	void* alloc_early_table();

	[[noreturn]] void mem_early_panic(const char* str);
}