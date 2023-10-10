/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include "Function.h"

namespace kstd {
	enum IterationAction {
		Continue,
		Break
	};

	template<typename... ArgTs>
	using IterationFunc = Function<IterationAction(ArgTs...)>;
};

#define ITER_RETVAL(expr) if((expr) == kstd::IterationAction::Break) return val;
#define ITER_RET(expr) if((expr) == kstd::IterationAction::Break) return;
#define ITER_BREAK(expr) if((expr) == kstd::IterationAction::Break) break;