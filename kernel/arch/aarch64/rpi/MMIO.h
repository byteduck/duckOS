/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include <kernel/memory/VMRegion.h>
#include "../aarch64util.h"
#include <kernel/memory/MemoryManager.h>

namespace RPi {
	class MMIO {
	public:
		static MMIO& inst();

		static constexpr size_t phys_base = 0x3F000000;
		static constexpr size_t phys_size = 0x01000000;

		template<typename T>
		inline static T peek(size_t offset) {
			// If we haven't initialized the MMU yet
			if (__builtin_expect(Aarch64::get_pc() < HIGHER_HALF, false) || !MM.is_paging_setup())
				return peek_early<T>(offset);

			return inst().get<T>(offset);
		}

		template<typename T>
		inline static void poke(size_t offset, T val) {
			// If we haven't initialized the MMU yet
			if (__builtin_expect(Aarch64::get_pc() < HIGHER_HALF, false) || !MM.is_paging_setup())
				return poke_early<T>(offset, val);

			inst().set<T>(offset, val);
		}


	private:
		MMIO();

		template<typename T>
		inline T get(size_t offset) {
			ASSERT(offset + sizeof(T) <= phys_size);
			return *((T*) (m_region->start() + offset));
		}

		template<typename T>
		inline void set(size_t offset, T val) {
			ASSERT(offset + sizeof(T) <= phys_size);
			*((T*) (m_region->start() + offset)) = val;
		}

		template<typename T>
		inline static T peek_early(size_t offset) {
			return *((T*) (phys_base + offset));
		}

		template<typename T>
		inline static void poke_early(size_t offset, T val) {
			*((T*) (phys_base + offset)) = val;
		}

		kstd::Arc<VMRegion> m_region;

		static MMIO* s_inst;
	};
}
