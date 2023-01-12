/*
	This file is part of duckOS.

	duckOS is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	duckOS is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with duckOS.  If not, see <https://www.gnu.org/licenses/>.

	Copyright (c) Byteduck 2016-2021. All rights reserved.
*/

#pragma once

#include "layout/BoxLayout.h"
#include "Label.h"
#include <string>
#include <functional>

namespace UI {
	enum class ButtonStyle {
		RAISED, FLAT, INSET
	};

	class Button: public Widget {
	public:
		WIDGET_DEF(Button)

		//Button
		[[nodiscard]] std::string label();
		void set_label(std::string new_label);
		void set_style(ButtonStyle new_style);

		//Widget
		virtual bool on_mouse_button(Pond::MouseButtonEvent evt) override;
		virtual void on_mouse_leave(Pond::MouseLeaveEvent evt) override;
		virtual bool on_mouse_move(Pond::MouseMoveEvent evt) override;
		virtual Gfx::Dimensions preferred_size() override;
		virtual void calculate_layout() override;

		std::function<void()> on_pressed = nullptr;
	private:
		explicit Button(std::string label);
		explicit Button(Duck::Ptr<const Gfx::Image> image);
		explicit Button(Duck::Ptr<Widget> contents);

		//Widget
		void do_repaint(const DrawContext& ctx) override;

		Duck::Ptr<Label> m_label;
		Duck::Ptr<Widget> m_contents;
		bool m_pressed = false;
		bool m_hovered = false;
		int m_padding = 4;
		ButtonStyle m_style = ButtonStyle::RAISED;
	};
}

