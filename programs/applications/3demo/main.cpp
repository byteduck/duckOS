/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include <libui/libui.h>
#include <libui/Window.h>
#include <lib3d/ViewportWidget.h>
#include <lib3d/ObjReader.h>
#include "DemoWidget.h"

int main(int argc, char** argv, char** envp) {
	UI::init(argv, envp);

	auto window = UI::Window::make();
	window->set_resizable(true);
	window->set_title("Lib3D Demo");

	Duck::Path obj;
	if (argc < 2)
		obj = UI::app_info().resource_path("cube.obj");
	else
		obj = argv[1];

	Duck::FileInputStream fis(obj);
	if (!fis.is_open()) {
		perror("3demo");
		return fis.status().code();
	}

	auto faces = Lib3D::ObjReader::read(fis);
	auto widget = DemoWidget::make(faces, argc < 2);


	window->set_contents(widget);
	window->set_resizable(false);
	window->set_decorated(true);
	window->show();

	UI::run();
}