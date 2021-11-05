//
// Created by aaron on 11/4/2021.
//

#include "Image.h"

using namespace UI;

Image::Image(const Gfx::Image& image, Image::ScalingMode mode): _image(image), _scaling_mode(mode) {
    set_uses_alpha(true);
}

const Gfx::Image& Image::image() {
    return _image;
}

void Image::set_image(const Gfx::Image& image) {
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

void Image::do_repaint(const DrawContext& ctx) {
    if(_scaling_mode == CENTER) {
        ctx.framebuffer().fill({0, 0, ctx.width(), ctx.height()}, RGBA(0, 0, 0, 0));
        ctx.framebuffer().draw_image(_image, {
            ctx.width() / 2 - _image.width / 2,
            ctx.height() / 2 - _image.height / 2
        });
        return;
    } else if(_scaling_mode == STRETCH) {
        ctx.framebuffer().draw_image_scaled(_image, {0, 0, ctx.width(), ctx.height()});
    } else if(_scaling_mode == FILL) {
        double ctx_ratio = (double) ctx.width() / (double) ctx.height();
        double image_ratio = (double) _image.width / (double) _image.height;
        if(ctx_ratio < image_ratio) {
            int scaled_width = (int) (_image.width * ((double) ctx.height() / (double) _image.height));
            ctx.framebuffer().draw_image_scaled(_image, {
                ctx.width() / 2 - scaled_width / 2,
                0,
                scaled_width,
                ctx.height()
            });
        } else {
            int scaled_height = (int) (_image.height * ((double) ctx.width() / (double) _image.width));
            ctx.framebuffer().draw_image_scaled(_image, {
                    0,
                    ctx.height() / 2 - scaled_height / 2,
                    ctx.width(),
                    scaled_height
            });
        }
    }
}

Dimensions Image::preferred_size() {
    return {_image.width, _image.height};
}
