/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include <libui/widget/Widget.h>
#include <libui/Timer.h>
#include <lib3d/ViewportWidget.h>

class DemoWidget: public UI::Widget {
public:
	WIDGET_DEF(DemoWidget);

	bool on_mouse_button(Pond::MouseButtonEvent evt) override;
	bool on_mouse_move(Pond::MouseMoveEvent evt) override;

private:
	DemoWidget(std::vector<std::array<Lib3D::Vertex, 3>> faces, bool texture);

	void change_texture();

	Duck::Ptr<Lib3D::RenderContext> context;
	Duck::Ptr<UI::Timer> timer;
	Duck::Ptr<Lib3D::ViewportWidget> viewport;
	Duck::Ptr<Lib3D::Texture> texture;
	std::vector<std::array<Lib3D::Vertex, 3>> faces;
	Vec3f rot;
	bool do_rot = true;
	unsigned int mouse = 0;
	std::string last_app = "";
	bool use_texture;
};
