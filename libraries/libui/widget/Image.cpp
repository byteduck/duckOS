//
// Created by aaron on 11/4/2021.
//

#include "Image.h"
#include "../libui.h"

using namespace UI;

Image::Image(Duck::Ptr<const Gfx::Image> image, Image::ScalingMode mode, Gfx::Dimensions preferred_size):
	_image(image), _scaling_mode(mode), _preferred_size(preferred_size) {
	set_uses_alpha(true);
}

Image::Image(std::string name, UI::Image::ScalingMode mode, Gfx::Dimensions preferred_size):
	_image(UI::icon(name)), _scaling_mode(mode), _preferred_size(preferred_size) {
	set_uses_alpha(true);
}

Duck::Ptr<const Gfx::Image> Image::image() {
	return _image;
}

void Image::set_image(Duck::Ptr<const Gfx::Image> image) {
	_image = image;
	repaint();
	update_layout();
}

Image::ScalingMode Image::scaling_mode() {
	return _scaling_mode;
}

void Image::set_scaling_mode(ScalingMode mode) {
	_scaling_mode = mode;
	repaint();
}

void Image::set_preferred_size(Gfx::Dimensions dimensions) {
	_preferred_size = dimensions;
}

void Image::do_repaint(const DrawContext& ctx) {
	ctx.framebuffer().fill({0, 0, ctx.width(), ctx.height()}, RGBA(0, 0, 0, 0));
	if(_scaling_mode == CENTER) {
		ctx.draw_image(_image, {
			ctx.width() / 2 - _image->size().width / 2,
			ctx.height() / 2 - _image->size().height / 2
		});
		return;
	} else if(_scaling_mode == STRETCH) {
		ctx.draw_image(_image, {0, 0, ctx.width(), ctx.height()});
	} else if(_scaling_mode == FILL || _scaling_mode == FIT) {
		double ctx_ratio = (double) ctx.width() / (double) ctx.height();
		double image_ratio = (double) _image->size().width / (double) _image->size().height;
		if(_scaling_mode == FILL ? ctx_ratio < image_ratio : ctx_ratio > image_ratio) {
			int scaled_width = (int) (_image->size().width * ((double) ctx.height() / (double) _image->size().height));
			ctx.draw_image(_image, {
				ctx.width() / 2 - scaled_width / 2,
				0,
				scaled_width,
				ctx.height()
			});
		} else {
			int scaled_height = (int) (_image->size().height * ((double) ctx.width() / (double) _image->size().width));
			ctx.draw_image(_image, {
					0,
					ctx.height() / 2 - scaled_height / 2,
					ctx.width(),
					scaled_height
			});
		}
	}
}

Gfx::Dimensions Image::preferred_size() {
	return _preferred_size.width > 0 ? _preferred_size : _image->size();
}
