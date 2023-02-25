/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once
#include <libsys/CPU.h>
#include "GraphModule.h"

class CPUModule: public GraphModule {
public:
	WIDGET_DEF(CPUModule);

protected:
	float plot_value() override;
	Gfx::Color graph_color() const override { return Gfx::Color(255, 50, 50); };

private:
	CPUModule();

	Duck::FileInputStream m_stream;
};
