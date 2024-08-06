/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include <kernel/time/TimeKeeper.h>

class ARMTimer: public TimeKeeper {
public:
	ARMTimer(TimeManager* manager);

	int frequency() override;
	void enable() override;
	void disable() override;
};
