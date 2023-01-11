/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "AppMenu.h"
#include <libui/widget/Image.h>
#include <libui/widget/Label.h>
#include <libui/widget/Button.h>

using namespace UI;
using namespace Duck;

AppMenu::AppMenu():
	m_window(Window::make()),
	m_layout(BoxLayout::make(BoxLayout::VERTICAL))
{
	m_window->set_contents(m_layout);
	m_window->set_decorated(false);
	m_window->set_resizable(false);
	auto apps = App::get_all_apps();
	for(auto app : apps) {
		if(app.hidden())
			continue;
		auto btn_layout = UI::BoxLayout::make(UI::BoxLayout::HORIZONTAL, 4);
		btn_layout->add_child(UI::Image::make(app.icon()));
		auto btn_label = UI::Label::make(app.name());
		btn_label->set_alignment(UI::CENTER, UI::BEGINNING);
		btn_layout->add_child(btn_label);
		auto btn = UI::Button::make(btn_layout);
		btn->set_sizing_mode(UI::PREFERRED);
		btn->on_pressed = [app, this]{
			app.run();
			m_window->hide();
			m_shown = false;
		};
		m_layout->add_child(btn);
	}
	m_window->resize(m_layout->preferred_size());
}

void AppMenu::show() {
	m_window->show();
	m_window->bring_to_front();
	m_shown = true;
}

void AppMenu::hide() {
	m_window->hide();
	m_shown = false;
}

void AppMenu::toggle() {
	if(m_shown)
		hide();
	else
		show();
}

Duck::Ptr<UI::Window> AppMenu::window() {
	return m_window;
}
