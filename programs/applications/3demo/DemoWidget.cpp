/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "DemoWidget.h"
#include <lib3d/RenderContext.h>
#include <lib3d/ObjReader.h>
#include <libui/libui.h>
#include <ctime>

DemoWidget::DemoWidget(std::vector<std::array<Lib3D::Vertex, 3>> object, bool texture):
	faces(object),
	use_texture(texture)
{
	context = Lib3D::RenderContext::make(Gfx::Dimensions {250, 250});
	viewport = Lib3D::ViewportWidget::make(context);

	add_child(viewport);
	viewport->set_window_draggable(true);

	float max_dim = -INFINITY;
	float min_dim = INFINITY;

	for(auto& face : faces) {
		for(auto& vert : face) {
			for(int i = 0; i < 3; i++) {
				max_dim = std::max(max_dim, vert.pos[i]);
				min_dim = std::min(min_dim, vert.pos[i]);
			}
		}
	}

	context->set_projmat(Lib3D::ortho(min_dim * 2, max_dim * 2, min_dim * 2, max_dim * 2, min_dim * 2, max_dim * 2));

	change_texture();

	timer = UI::set_interval([this] {
		context->clear({0, 0, 0});
		context->set_modelmat(Lib3D::rotate(0.006f, rot));
		for(auto& face : faces) {
			context->tri(face);
		}
		if (do_rot)
			rot += {1.234, 2.312, 3.231};
		viewport->repaint();
	}, 16);
}

bool DemoWidget::on_mouse_button(Pond::MouseButtonEvent evt) {
	mouse = evt.new_buttons;
	if (!(evt.old_buttons & POND_MOUSE2) && (evt.new_buttons & POND_MOUSE2)) {
		open_menu(UI::Menu::make({
			UI::MenuItem::make("Toggle decoration", [this] {
				root_window()->set_uses_alpha(root_window()->decorated());
				root_window()->set_decorated(!root_window()->decorated());
			}),
			UI::MenuItem::make("Change Texture", [this] { change_texture(); })
		}));
		return true;
	}
	return false;
}

bool DemoWidget::on_mouse_move(Pond::MouseMoveEvent evt) {
	return false;
}

void DemoWidget::change_texture() {
	if (!use_texture)
		return;
	srand(std::time(nullptr));
	auto apps = App::get_all_apps();
	App::Info app;
	do {
		app = apps[rand() % apps.size()];
	} while(app.name() == last_app);
	last_app = app.name();
	texture = Duck::Ptr<Lib3D::Texture>(new Lib3D::Texture(*app.icon()->framebuffer()));
	context->bind_texture(texture.get());
}
