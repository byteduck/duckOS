//
// Created by aaron on 11/4/2021.
//

#pragma once

#include "Widget.h"

namespace UI {
	class Image: public Widget {
	public:
		WIDGET_DEF(Image)

		enum ScalingMode {
			STRETCH,
			CENTER,
			FILL,
			FIT
		};

		//Image
		Duck::Ptr<const Gfx::Image> image();
		void set_image(Duck::Ptr<const Gfx::Image> image);
		ScalingMode scaling_mode();
		void set_scaling_mode(ScalingMode mode);
		void set_preferred_size(Gfx::Dimensions dimensions);

		//Widget
		void do_repaint(const UI::DrawContext& ctx) override;
		Gfx::Dimensions preferred_size() override;

	private:
		explicit Image(Duck::Ptr<const Gfx::Image> image, ScalingMode mode = FIT, Gfx::Dimensions preferred_size = {0, 0});
		explicit Image(std::string name, ScalingMode mode = FIT, Gfx::Dimensions preferred_size = {0, 0});

		Duck::Ptr<const Gfx::Image> _image;
		ScalingMode _scaling_mode;
		Gfx::Dimensions _preferred_size = {0, 0};
	};
}

