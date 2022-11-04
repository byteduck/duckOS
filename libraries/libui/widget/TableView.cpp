/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "TableView.h"
#include "layout/BoxLayout.h"

using namespace UI;

class TableViewCell: public Widget {
public:
	WIDGET_DEF(TableViewCell)

	Gfx::Dimensions preferred_size() override {
		return m_preferred_size;
	}

	void do_repaint(const DrawContext& ctx) override {
		ctx.fill(ctx.rect(), m_color);
	}

private:
	TableViewCell(Gfx::Dimensions size, Color color, Duck::Ptr<Widget> widget): m_preferred_size(size), m_color(color) {
		set_uses_alpha(true);
		widget->set_sizing_mode(UI::FILL);
		add_child(widget);
	}

	Gfx::Dimensions m_preferred_size;
	Color m_color;
};

TableView::TableView(int num_cols): m_num_cols(num_cols) {
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
	auto delegate = m_delegate.lock();

	auto layout = BoxLayout::make(BoxLayout::HORIZONTAL);
	int total_width = 0;
	for(int i = 0; i < m_num_cols; i++) {
		auto width = (i == m_num_cols - 1) ? current_size().width - 12 - total_width : m_column_widths[i];
		width = std::max(1, width);
		total_width += width;
		auto color = index % 2 == 0 ? Theme::shadow_1() : Theme::shadow_2();
		layout->add_child(TableViewCell::make(Gfx::Dimensions{width, m_row_height}, color, delegate->tv_create_entry(index, i)));
	}
	return layout;
}

Gfx::Dimensions TableView::lv_preferred_item_dimensions() {
	int sum = 0;
	for(auto col : m_column_widths)
		if(col != -1)
			sum += col;
	return { sum, m_row_height };
}

int TableView::lv_num_items() {
	if(m_delegate.expired())
		return 0;
	return m_delegate.lock()->tv_num_entries();
}

void TableView::calculate_layout() {
	auto dims = current_size();
	m_list_view->set_layout_bounds({0, 16, dims.width, dims.height - 16});
	update_data();
}

void TableView::do_repaint(const DrawContext& ctx) {
	int x = 0;
	for(int col = 0; col < m_num_cols; col++) {
		auto width = (col == m_num_cols - 1) ? current_size().width - x : m_column_widths[col];
		auto col_rect = Gfx::Rect {x, 0, width, 16};
		ctx.draw_outset_rect(col_rect, Theme::bg());
		ctx.draw_text(m_header_labels[col].c_str(), col_rect.inset(0, 4, 0, 4), BEGINNING, CENTER, Theme::font(), Theme::fg());
		x += width;
	}
}

Gfx::Dimensions TableView::preferred_size() {
	return m_list_view->preferred_size() + Gfx::Dimensions {0, 16};
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
