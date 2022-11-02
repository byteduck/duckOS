/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "MenuWidget.h"
#include <libgraphics/Font.h>

#define MAX_MENU_WIDTH 200
#define ITEM_HEIGHT 20
#define ITEM_PADDING_X 4
#define ITEM_PADDING_Y 2
#define MENU_PADDING_Y 2
#define MENU_PADDING_RIGHT 16
#define SEPARATOR_ITEM_HEIGHT 5

using namespace UI;


MenuWidget::MenuWidget(Duck::Ptr<Menu> menu, Duck::PtrRef<Window> window): m_menu(menu), m_window(window) {}

void MenuWidget::initialize() {
	auto window = m_window.lock();
	window->delegate = self<WindowDelegate>();
}

bool MenuWidget::on_mouse_move(Pond::MouseMoveEvent evt) {
	if(evt.new_pos.y >= MENU_PADDING_Y) {
		Duck::Ptr<MenuItem> hovered_item;
		Gfx::Rect cur_rect = {0, MENU_PADDING_Y, current_size().width, 0};
		for(auto& item : m_menu->items()) {
			cur_rect.height = item == MenuItem::Separator ? SEPARATOR_ITEM_HEIGHT : ITEM_HEIGHT;
			if(evt.new_pos.in(cur_rect)) {
				hovered_item = item;
				break;
			}
			cur_rect.y += cur_rect.height;
		}
		if(hovered_item != m_hovered_item)
			repaint();
		if(hovered_item != m_last_hovered_item) {
			if(hovered_item && hovered_item->submenu) {
				m_expanded_item = hovered_item;
				open_child_window(hovered_item->submenu, cur_rect);
			} else {
				m_expanded_item = nullptr;
				if(m_child_menu.lock()) {
					m_child_menu.lock()->close();
				}
			}
		}
		m_hovered_item = hovered_item;
		m_last_hovered_item = hovered_item;
	}
	return true;
}

void MenuWidget::on_mouse_leave(Pond::MouseLeaveEvent evt) {
	m_hovered_item = nullptr;
	repaint();
}

bool MenuWidget::on_mouse_button(Pond::MouseButtonEvent evt) {
	if((evt.old_buttons & POND_MOUSE1) && !(evt.new_buttons & POND_MOUSE1) && m_hovered_item != MenuItem::Separator) {
		if(m_hovered_item && m_hovered_item->action)
			m_hovered_item->action();
		root_menu().lock()->close();
		return true;
	}
	return false;
}

Gfx::Dimensions MenuWidget::preferred_size() {
	int max_width = 0;
	int height = MENU_PADDING_Y * 2;
	for(auto& item : m_menu->items()) {
		if(item == MenuItem::Separator) {
			height += SEPARATOR_ITEM_HEIGHT;
			continue;
		}
		max_width = std::max(max_width, UI::Theme::font()->size_of(item->title.c_str()).width);
		height += ITEM_HEIGHT;
	}
	return {
		std::min(max_width + MENU_PADDING_RIGHT, MAX_MENU_WIDTH) + ITEM_PADDING_X * 2,
		height
	};
}

Duck::Ptr<MenuWidget> MenuWidget::open_menu(Duck::Ptr<Menu> menu, Gfx::Point location) {
	auto window = UI::Window::make();
	window->pond_window()->set_type(Pond::MENU);
	auto menu_widget = UI::MenuWidget::make(menu, window);
	window->set_contents(menu_widget);
	window->set_decorated(false);
	window->set_position(location);
	window->show();
	window->pond_window()->focus();
	return menu_widget;
}

void MenuWidget::window_focus_changed(const std::shared_ptr<Window>& window, bool focused) {
	auto root = root_menu().lock();
	if(!root->any_are_focused())
		root->close();
}

void MenuWidget::do_repaint(const DrawContext& ctx) {
	ctx.draw_outset_rect({0, 0, ctx.width(), ctx.height()});
	Gfx::Rect item_rect = {0, MENU_PADDING_Y, ctx.width(), ITEM_HEIGHT};
	for(auto& item : m_menu->items()) {
		if(item == MenuItem::Separator) {
			Gfx::Rect separator_rect = {ITEM_PADDING_X, item_rect.y + SEPARATOR_ITEM_HEIGHT / 2, ctx.width() - ITEM_PADDING_X * 2, 1};
			ctx.fill(separator_rect, UI::Theme::fg());
			item_rect.y += SEPARATOR_ITEM_HEIGHT;
			continue;
		}

		auto contents_rect = item_rect.inset(ITEM_PADDING_Y, ITEM_PADDING_X + 1, ITEM_PADDING_Y, ITEM_PADDING_X);
		if(m_hovered_item == item || m_expanded_item == item)
			ctx.fill(item_rect.inset(1, 2, 2, 1), UI::Theme::accent());
		ctx.draw_text(item->title.c_str(), contents_rect, BEGINNING, CENTER, UI::Theme::font(), UI::Theme::fg());
		if(item->submenu)
			ctx.draw_text(">", contents_rect, END, CENTER, UI::Theme::font(), UI::Theme::fg());
		item_rect.y += ITEM_HEIGHT;
	}
}

void MenuWidget::open_child_window(Duck::Ptr<Menu> item, Gfx::Rect item_rect) {
	if(m_child_menu.lock())
		m_child_menu.lock()->close();
	auto window_position = item_rect.position() + Gfx::Point {item_rect.width, -MENU_PADDING_Y} + root_window()->position();
	m_child_menu = open_menu(item, window_position);
	m_child_menu.lock()->m_parent = self();
}

Duck::WeakPtr<MenuWidget> MenuWidget::root_menu() {
	if(!m_parent.lock())
		return self();
	else
		return m_parent.lock()->root_menu();
}

bool MenuWidget::any_are_focused() {
	auto window = m_window.lock();
	if(window) {
		if(window->is_focused())
			return true;
		auto child = m_child_menu.lock();
		if(child)
			return child->m_window.lock()->is_focused();
	}
	return false;
}

void MenuWidget::close() {
	if(m_child_menu.lock())
		m_child_menu.lock()->close();
	if(m_window.lock())
		m_window.lock()->close();
}