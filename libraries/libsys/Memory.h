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

#ifndef DUCKOS_LIBSYS_MEMORY_H
#define DUCKOS_LIBSYS_MEMORY_H

#include <sys/types.h>
#include <iostream>
#include <libduck/Result.hpp>

namespace Sys::Mem {
	class Amount {
	public:
		static const size_t KiB = 1024;
		static const size_t MiB = 1048576;
		static const size_t GiB = 1073742000;

		size_t bytes;
		std::string readable() const;

		inline double kib() const {
			return bytes / (double) KiB;
		}

		inline double mib() const {
			return bytes / (double) MiB;
		}

		inline double gib() const {
			return bytes / (double) GiB;
		}

		inline operator size_t() const {
			return bytes;
		}
	};

	class Info {
	public:
		Amount usable;
		Amount used;
		Amount reserved;
		Amount kernel_virt;
		Amount kernel_phys;
		Amount kernel_heap;
		Amount kernel_disk_cache;

		inline double used_frac() const {
			return (double)((long double) used / (long double) usable);
		}

		inline double free_frac() const {
			return 1.0 - used_frac();
		}

		inline Amount free() const {
			return {(size_t) usable - (size_t) used};
		}
	};

	ResultRet<Info> get_info(std::istream& file);
	ResultRet<Info> get_info();
}
#endif //DUCKOS_LIBSYS_MEMORY_H
