/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include "layout/BoxLayout.h"

namespace UI {
	class Stack: public BoxLayout {
	public:
		WIDGET_DEF(Stack)

		using Direction = BoxLayout::Direction;

	protected:
		template<typename... WidgetTs>
		Stack(Direction direction, int spacing, WidgetTs... widgets):
			BoxLayout(direction, spacing),
			m_widgets({widgets...})
		{}

		template<typename... WidgetTs>
		Stack(Direction direction, WidgetTs... widgets):
				BoxLayout(direction),
				m_widgets({widgets...})
		{}

		void initialize() override;

	private:
		std::vector<Duck::Ptr<Widget>> m_widgets;
	};
}