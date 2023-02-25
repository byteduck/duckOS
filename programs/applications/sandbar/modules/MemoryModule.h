/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once
#include <libsys/Memory.h>
#include "GraphModule.h"

class MemoryModule: public GraphModule {
public:
	WIDGET_DEF(MemoryModule);

protected:
	float plot_value() override;
	Gfx::Color graph_color() const override { return UI::Theme::accent(); };

private:
	MemoryModule();

	Duck::FileInputStream m_stream;
};