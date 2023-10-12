/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include "Widget.h"

namespace UI {
	class NamedCell: public Widget {
	public:
		WIDGET_DEF(NamedCell);

	protected:
		Gfx::Dimensions preferred_size() override;
		Gfx::Dimensions minimum_size() override;
		void calculate_layout() override;
		void do_repaint(const UI::DrawContext& ctx) override;

	private:
		NamedCell(std::string name, Duck::Ptr<UI::Widget> contents);

		void initialize() override;

		Duck::Ptr<UI::Widget> m_contents;
		std::string m_name;
		int m_padding;
	};
}
