/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "Widget.h"
#include "layout/FlexLayout.h"
#include "ListView.h"

namespace UI {
	class TableViewDelegate {
	public:
		virtual Duck::Ptr<Widget> tv_create_entry(int row, int col) = 0;
		virtual std::string tv_column_name(int col) = 0;
		virtual int tv_num_entries() = 0;
		virtual int tv_row_height() = 0;
		virtual int tv_column_width(int col) = 0;
	};

	class TableView: public Widget, public ListViewDelegate {
	public:
		WIDGET_DEF(TableView)

		// TableView
		void update_row(int row);
		void update_data();
		void set_delegate(Duck::Ptr<TableViewDelegate> delegate);

	protected:
		// ListViewDelegate
		Duck::Ptr<Widget> lv_create_entry(int index) override;
		Gfx::Dimensions lv_preferred_item_dimensions() override;
		int lv_num_items() override;

		// Widget
		void calculate_layout() override;
		void do_repaint(const DrawContext& ctx) override;
		Gfx::Dimensions preferred_size() override;

		// Object
		void initialize() override;

	private:
		TableView(int num_cols);

		void update_columns();

		Duck::Ptr<ListView> m_list_view = ListView::make(ListView::VERTICAL);
		const int m_num_cols;
		Duck::WeakPtr<TableViewDelegate> m_delegate;
		std::vector<std::string> m_header_labels;
		std::vector<int> m_column_widths;
		int m_row_height = 1;
	};
}

