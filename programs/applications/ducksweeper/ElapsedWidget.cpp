#include "ElapsedWidget.h"

ElapsedWidget::ElapsedWidget() {
    this->elapsed = 0;
    this->elapsed_label = UI::Label::make("0");
	this->timer = UI::set_interval([&]() {
		this->elapsed += 1;
		this->elapsed_label->set_label(std::to_string(this->elapsed));
	}, 1000);


    add_child(elapsed_label);
}

void ElapsedWidget::reset() {
    this->elapsed = 0;
    this->elapsed_label->set_label("0");
    this->timer->start();
    repaint();
}

void ElapsedWidget::stop() {
    this->timer->stop();
}

Gfx::Dimensions ElapsedWidget::preferred_size() {
    return {20, 20};
}

void ElapsedWidget::do_repaint(const UI::DrawContext& ctx) {
    Gfx::Rect rect = {0, 0, ctx.width(), ctx.height()};
    ctx.fill(rect, UI::Theme::bg());
    ctx.draw_text(std::to_string(this->elapsed).c_str(),
                 rect,
                 UI::TextAlignment::CENTER, UI::TextAlignment::CENTER, UI::Theme::font(),
                 RGBA(255,255,255,255));
}