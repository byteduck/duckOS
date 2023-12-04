/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "Widget.h"
#include "layout/FlexLayout.h"
#include "ListView.h"
#include "layout/BoxLayout.h"

namespace UI {
	enum TableViewSelectionMode {
		NONE,
		SINGLE
	};

	class TableViewCell;
	class TableViewRow;

	class TableViewDelegate {
	public:
		virtual Duck::Ptr<Widget> tv_create_entry(int row, int col) = 0;
		virtual std::string tv_column_name(int col) = 0;
		virtual int tv_num_entries() = 0;
		virtual int tv_row_height() = 0;
		virtual int tv_column_width(int col) = 0;
		virtual TableViewSelectionMode tv_selection_mode() { return SINGLE; }
		virtual void tv_selection_changed(const std::set<int>& selection) {}
		virtual Duck::Ptr<Menu> tv_entry_menu(int row) { return nullptr; }
	};

	class TableView: public Widget, public ListViewDelegate {
	public:
		WIDGET_DEF(TableView)

		// TableView
		void update_row(int row);
		void update_data();
		void set_delegate(Duck::Ptr<TableViewDelegate> delegate);

	protected:
		// TableView
		friend class TableViewCell;
		friend class TableViewRow;
		bool row_clicked(Duck::Ptr<UI::TableViewRow> row, Pond::MouseButtonEvent evt);

		// ListViewDelegate
		Duck::Ptr<Widget> lv_create_entry(int index) override;
		Gfx::Dimensions lv_preferred_item_dimensions() override;
		int lv_num_items() override;

		// Widget
		void calculate_layout() override;
		void do_repaint(const DrawContext& ctx) override;
		Gfx::Dimensions preferred_size() override;
		Gfx::Dimensions minimum_size() override;

		// Object
		void initialize() override;

	private:
		TableView(int num_cols, bool shows_tabs);

		void update_columns();
		std::vector<int> calculate_column_widths();
		void make_row_selection(Duck::Ptr<TableViewRow> row);
		void open_row_menu(Duck::Ptr<TableViewRow> row);

		static constexpr int c_padding_tl = 2;
		static constexpr int c_padding_br = 1;
		static constexpr int c_tab_height = 16;

		Duck::Ptr<ListView> m_list_view = ListView::make(ListView::VERTICAL);
		const int m_num_cols;
		Duck::WeakPtr<TableViewDelegate> m_delegate;
		std::vector<std::string> m_header_labels;
		std::vector<int> m_column_widths;
		int m_row_height = 1;
		std::set<int> m_selected_items;
		bool m_show_tabs = true;
	};

	class TableViewCell: public Widget {
	public:
		WIDGET_DEF(TableViewCell)
		Gfx::Dimensions preferred_size() override;

	private:
		friend class TableView;
		TableViewCell(Gfx::Dimensions size, Duck::Ptr<Widget> widget);
		Gfx::Dimensions m_preferred_size;
	};

	class TableViewRow: public BoxLayout {
	public:
		WIDGET_DEF(TableViewRow);

	protected:
		bool on_mouse_button(Pond::MouseButtonEvent evt) override;
		void do_repaint(const DrawContext& ctx) override;

	private:
		friend class TableView;
		TableViewRow(Gfx::Color color, Duck::Ptr<TableView> table_view, int row);
		bool selected();

		Gfx::Color m_color;
		int m_row;
		Duck::WeakPtr<TableView> m_table_view;
	};
}

