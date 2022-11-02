/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include <libui/widget/layout/FlexLayout.h>
#include <libui/widget/files/FileViewDelegate.h>
#include <libui/widget/files/FileViewBase.h>
#include <libui/widget/Label.h>
#include <libui/widget/Button.h>

namespace UI {
	class FileNavigationBar : public UI::FlexLayout, public UI::FileViewDelegate {
	public:
		WIDGET_DEF(FileNavigationBar)

		void fv_did_navigate(Duck::Path path) override;

	private:
		FileNavigationBar(Duck::Ptr<UI::FileViewBase> file_view);

		Duck::Ptr<UI::FileViewBase> file_view;
		Duck::Ptr<UI::Button> back_button;
		Duck::Ptr<UI::Label> location_label;
	};
}
