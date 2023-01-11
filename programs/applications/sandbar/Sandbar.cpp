/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "Sandbar.h"
#include <libui/libui.h>


Sandbar::Sandbar() {
	// Make sandbar window
	auto window = UI::Window::make();
	window->set_title("Sandbar");
	window->set_decorated(false);
	window->set_resizable(false);
	window->pond_window()->set_draggable(false);

	// Get display dimensions
	auto dims = UI::pond_context->get_display_dimensions();

	// Make app menu
	m_app_menu = AppMenu::make();

	// Make sandbar widget
	m_widget = SandbarWidget::make(m_app_menu);
	window->set_contents(m_widget);

	// Position window at bottom of screen and show
	window->set_position({0, dims.height - window->dimensions().height});
	window->resize({dims.width, window->dimensions().height});
	window->show();

	m_app_menu->window()->set_position({0, dims.height - m_app_menu->window()->dimensions().height - window->dimensions().height});
}