/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "Stack.h"

using namespace UI;

void Stack::initialize() {
	for(auto& widget : m_widgets)
		add_child(widget);
	m_widgets.clear();
}
