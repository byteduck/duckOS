/*
	This file is part of duckOS.

	duckOS is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	duckOS is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with duckOS.  If not, see <https://www.gnu.org/licenses/>.

	Copyright (c) Byteduck 2016-2021. All rights reserved.
*/

#pragma once

#include <kernel/kstd/unix_types.h>
#include "pci/PCI.h"
#include "kstd/Arc.h"
#include "memory/VMRegion.h"

namespace IO {
	void wait();
	void outb(uint16_t port, uint8_t value);
	void outw(uint16_t port, uint16_t value);
	void outl(uint16_t port, uint32_t value);
	uint8_t inb(uint16_t port);
	uint16_t inw(uint16_t port);
	uint32_t inl(uint16_t port);
	inline void wait(size_t us) {
		while(us--)
			inb(0x80);
	}

	class Window {
	public:
		enum Type {
			Invalid, Mem16, Mem32, Mem64, IOSpace
		};

		Window() = default;
		Window(PCI::Address addr, uint8_t bar);

		template<typename T>
		T in(size_t offset) {
			ASSERT(m_type != Invalid);
			if (m_type == Type::IOSpace) {
				if constexpr(sizeof(T) == 1)
					return inb(m_addr + offset);
				else if constexpr(sizeof(T) == 2)
					return inw(m_addr + offset);
				else if constexpr(sizeof(T) == 4)
					return inl(m_addr + offset);
				static_assert(sizeof(T) <= 4 && sizeof(T) != 3);
			} else {
				return *((T*) (m_vm_region->start() + offset));
			}
		}

		uint8_t in8(size_t offset) { return in<uint8_t>(offset); }
		uint16_t in16(size_t offset) { return in<uint16_t>(offset); }
		uint32_t in32(size_t offset) { return in<uint32_t>(offset); }

		template<typename T>
		void out(size_t offset, T& data) {
			ASSERT(m_type != Invalid);
			if (m_type == Type::IOSpace) {
				if constexpr(sizeof(T) == 1)
					outb(m_addr + offset, data);
				else if constexpr(sizeof(T) == 2)
					outw(m_addr + offset, data);
				else if constexpr(sizeof(T) == 4)
					outl(m_addr + offset, data);
				static_assert(sizeof(T) <= 4 && sizeof(T) != 3);
			} else {
				*((T*) (m_vm_region->start() + offset)) = data;
			}
		}

		void out8(size_t offset, uint8_t val) { return out<uint8_t>(offset, val); }
		void out16(size_t offset, uint16_t val) { return out<uint16_t>(offset, val); }
		void out32(size_t offset, uint32_t val) { return out<uint32_t>(offset, val); }

	private:
		Type m_type = Invalid;
		size_t m_size = 0;
		size_t m_addr = 0;
		kstd::Arc<VMRegion> m_vm_region;
		bool m_prefetchable = false;
	};
};


