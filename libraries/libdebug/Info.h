/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include <string>
#include <libexec/Object.h>

namespace Debug {
	struct AddressInfo {
		std::string symbol_name;
		size_t symbol_offset;
		Exec::Object* object;
	};
}
