/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include <sys/types.h>
#include <string>

namespace Duck {
	struct DataSize {
	public:
		static constexpr size_t KiB = 1024;
		static constexpr size_t MiB = 1048576;
		static constexpr size_t GiB = 1073742000;

		enum class Suffix { Short, Long };
		enum class Precision { Round, Precise };

		size_t bytes;

		DataSize(): bytes(0) {}
		DataSize(size_t size): bytes(size) {}

		std::string readable(Precision precision = Precision::Round, Suffix suffix = Suffix::Short) const;

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
}