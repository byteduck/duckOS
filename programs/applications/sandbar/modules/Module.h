/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include <libui/widget/Widget.h>

class Module: public UI::Widget {
public:
	virtual void update() = 0;
};
