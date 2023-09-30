/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include "Object.h"

namespace Duck {
	class Buffer: public Object {
	public:
		DUCK_OBJECT_VIRTUAL(Buffer<T>);

		[[nodiscard]] virtual size_t size() const = 0;
		[[nodiscard]] virtual void* data() const = 0;

	};
}
