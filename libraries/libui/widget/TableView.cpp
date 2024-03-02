/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "TableView.h"
#include "layout/BoxLayout.h"

using namespace UI;

TableView::TableView(int num_cols, bool shows_tabs):
	m_num_cols(num_cols),
	m_show_tabs(shows_tabs)
{
	set_sizing_mode(UI::FILL);
	for(int i = 0; i < num_cols; i++) {
		m_header_labels.push_back("");
		m_column_widths.push_back(1);
	}
	add_child(m_list_view);
}

void TableView::update_row(int row) {
	m_list_view->update_item(row);
}

void TableView::update_data() {
	update_columns();
	m_list_view->update_data();
}

void TableView::set_delegate(Duck::Ptr<TableViewDelegate> delegate) {
	m_delegate = delegate;
	update_data();
}

Duck::Ptr<Widget> TableView::lv_create_entry(int index) {
	if(m_delegate.expired())
		return nullptr;

	auto color = index % 2 == 0 ? Theme::shadow_1() : Theme::shadow_2();
	auto row = TableViewRow::make(color, self(), index);
	auto delegate = m_delegate.lock();
	auto widths = calculate_column_widths();

	for(int i = 0; i < m_num_cols; i++) {
		row->add_child(TableViewCell::make(
			Gfx::Dimensions{widths[i], m_row_height},
			delegate->tv_create_entry(index, i)
		));
	}

	return row;
}

Gfx::Dimensions TableView::lv_preferred_item_dimensions() {
	int sum = 0;
	for(auto col : m_column_widths)
		if(col != -1)
			sum += col;
		else
			sum += 100;
	return { sum, m_row_height };
}

int TableView::lv_num_items() {
	if(m_delegate.expired())
		return 0;
	return m_delegate.lock()->tv_num_entries();
}

void TableView::calculate_layout() {
	auto dims = current_size();
	auto tab_height = m_show_tabs ? c_tab_height : 0;
	m_list_view->set_layout_bounds({
		c_padding_tl,
		tab_height + c_padding_tl,
		dims.width - c_padding_tl - c_padding_br,
		dims.height - tab_height - c_padding_tl - c_padding_br});
	update_data();
}

void TableView::do_repaint(const DrawContext& ctx) {
	ctx.draw_inset_outline(ctx.rect());
	if (!m_show_tabs)
		return;
	int x = c_padding_tl;
	auto widths = calculate_column_widths();
	for(int col = 0; col < m_num_cols; col++) {
		auto width = col == m_num_cols - 1 ? current_size().width - c_padding_br - x : widths[col];
		auto col_rect = Gfx::Rect {x, c_padding_tl, width, c_tab_height};
		ctx.draw_outset_rect(col_rect, Theme::bg());
		ctx.draw_text(m_header_labels[col].c_str(), col_rect.inset(0, 4, 0, 4), BEGINNING, CENTER, Theme::font(), Theme::fg());
		x += width;
	}
}

Gfx::Dimensions TableView::preferred_size() {
	auto tab_height = m_show_tabs ? c_tab_height : 0;
	auto out = m_list_view->preferred_size() + Gfx::Dimensions { c_padding_tl + c_padding_br, c_padding_tl + c_padding_br + tab_height};
	out.height = std::min(out.height, 300);
	return out;
}

Gfx::Dimensions TableView::minimum_size() {
	auto tab_height = m_show_tabs ? c_tab_height : 0;
	return m_list_view->minimum_size() + Gfx::Dimensions {c_padding_tl + c_padding_br, c_padding_tl + c_padding_br + tab_height};
}

void TableView::initialize() {
	Widget::initialize();
	m_list_view->delegate = self();
}

void TableView::update_columns() {
	if(m_delegate.expired())
		return;
	auto delegate = m_delegate.lock();
	for(int i = 0; i < m_num_cols; i++) {
		m_header_labels[i] = delegate->tv_column_name(i);
		m_column_widths[i] = delegate->tv_column_width(i);
	}
	m_row_height = delegate->tv_row_height();
}

std::vector<int> TableView::calculate_column_widths() {
	// Count stretchy columns
	int total_width = 0;
	int num_stretchy = 0;
	int stretchy_width = 0;
	for(auto& width : m_column_widths) {
		if(width != -1)
			total_width += width;
		else
			num_stretchy++;
	}

	// Figure out width of stretchy columns
	auto remaining_width = current_size().width - 12 - c_padding_tl - c_padding_br - total_width;
	if(num_stretchy)
		stretchy_width = std::max(16, remaining_width / num_stretchy);

	// Add 1px to some stretchy columns if needed to make up for remainder
	int stretchy_remainder = num_stretchy ? remaining_width % num_stretchy : 0;

	// Calculate widths and return
	std::vector<int> ret = m_column_widths;
	for(auto& width : ret)
		if(width == -1)
			width = stretchy_width + (std::max(stretchy_remainder--, 0) ? 1 : 0);

	return ret;
}

bool TableView::row_clicked(Duck::Ptr<TableViewRow> row, Pond::MouseButtonEvent evt) {
	auto delegate = m_delegate.lock();
	if (!delegate)
		return false;

	if (!(evt.old_buttons & POND_MOUSE1) && (evt.new_buttons & POND_MOUSE1)) {
		make_row_selection(row);
		return true;
	} else if (!(evt.old_buttons & POND_MOUSE2) && (evt.new_buttons & POND_MOUSE2)) {
		make_row_selection(row);
		open_row_menu(row);
		return true;
	}

	return false;
}

void TableView::open_row_menu(Duck::Ptr<TableViewRow> row) {
	auto delegate = m_delegate.lock();
	auto menu = delegate->tv_entry_menu(row->m_row);
	if (menu)
		open_menu(menu);
}

void TableView::make_row_selection(Duck::Ptr<TableViewRow> row) {
	auto delegate = m_delegate.lock();
	auto mode = delegate->tv_selection_mode();
	if (mode == NONE)
		return;

	m_selected_items.clear();
	m_selected_items.insert(row->m_row);
	delegate->tv_selection_changed(m_selected_items);

	for (auto& child : m_list_view->get_children())
		child->repaint();
}

/** TableViewCell **/

TableViewCell::TableViewCell(Gfx::Dimensions size, Duck::Ptr<Widget> widget):
		m_preferred_size(size)
{
	set_uses_alpha(true);
	widget->set_sizing_mode(UI::FILL);
	add_child(widget);
}

Gfx::Dimensions TableViewCell::preferred_size()  {
	return m_preferred_size;
}

/** TableViewRow **/

TableViewRow::TableViewRow(Gfx::Color color, Duck::Ptr<TableView> table_view, int row):
	BoxLayout(HORIZONTAL),
	m_color(color),
	m_table_view(table_view),
	m_row(row)
{}

void TableViewRow::do_repaint(const DrawContext& ctx) {
	if (selected())
		ctx.fill(ctx.rect(), UI::Theme::accent());
	else
		ctx.fill(ctx.rect(), m_color);
}

bool TableViewRow::on_mouse_button(Pond::MouseButtonEvent evt) {
	auto table = m_table_view.lock();
	if (!table)
		return false;
	return table->row_clicked(self(), evt);
}

bool TableViewRow::selected() {
	auto& selected_items = m_table_view.lock()->m_selected_items;
	return selected_items.find(m_row) != selected_items.end();
}
