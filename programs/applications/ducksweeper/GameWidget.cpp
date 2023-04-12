#include "GameWidget.h"

GameWidget::GameWidget()
{
    auto app_info = App::Info::from_current_app();
    if (app_info.is_error())
    {
        Duck::Log::errf("could not load app info: {}", app_info.result());
    }
    auto base_path = app_info.value().base_path();
    auto flag_file = Gfx::Image::load(base_path / "flag.png");
    if (flag_file.is_error())
    {
        Duck::Log::errf("flag file error: {}", flag_file.result());
        return;
    }
    flag = flag_file.value();

    auto duck_file = Gfx::Image::load(base_path / "duck.png");
    if (duck_file.is_error())
    {
        Duck::Log::errf("duck file error: {}", duck_file.result());
    }
    duck = duck_file.value();
    reset();
}

void GameWidget::reset()
{
    board.reset_board();
    repaint();
}

void GameWidget::handle_stop()
{
    if (on_stop)
        on_stop();
}

Gfx::Dimensions GameWidget::preferred_size()
{
    return {board.columns() * (CELL_SIZE + 1) + 4, board.rows() * (CELL_SIZE + 1) + 4};
}

bool GameWidget::on_mouse_move(Pond::MouseMoveEvent evt)
{
    int new_hovered_cell_x = evt.new_pos.x / (CELL_SIZE + 1);
    int new_hovered_cell_y = evt.new_pos.y / (CELL_SIZE + 1);

    this->hover_x = new_hovered_cell_x;
    this->hover_y = new_hovered_cell_y;
    repaint();
    return true;
}

bool GameWidget::on_mouse_button(Pond::MouseButtonEvent evt)
{
    this->click_x = this->hover_x;
    this->click_y = this->hover_y;
    if ((evt.new_buttons & POND_MOUSE1) && !(evt.old_buttons & POND_MOUSE1))
    {
        board.reveil_cell(this->click_x, this->click_y);
    }
    else if ((evt.new_buttons & POND_MOUSE2) && !(evt.old_buttons & POND_MOUSE2))
    {
        board.mark_cell(this->click_x, this->click_y);
    }
    if (!board.can_move())
    {
        this->handle_stop();
    }
    repaint();
    return true;
}

void GameWidget::do_repaint(const UI::DrawContext &ctx)
{
    ctx.fill({0, 0, ctx.width(), ctx.height()}, CELL_COLOR);
    for (int i = 0; i < board.columns(); i++)
    {
        for (int j = 0; j < board.rows(); j++)
        {
            if (!board.is_visible(i, j))
            {
                ctx.fill({i * (CELL_SIZE + 1), j * (CELL_SIZE + 1), CELL_SIZE, CELL_SIZE}, CELL_FILL);
            }
            else
            {
                if (board.is_marked(i, j))
                {
                    ctx.fill({i * (CELL_SIZE + 1), j * (CELL_SIZE + 1), CELL_SIZE, CELL_SIZE}, CELL_FILL);
                    ctx.draw_image(flag, {i * (CELL_SIZE + 1) + (CELL_SIZE - IMAGE_SIZE) / 2, j * (CELL_SIZE + 1) + (CELL_SIZE - IMAGE_SIZE) / 2});
                }
                else if (board.is_mine(i, j))
                {
                    if (i == this->click_x && j == this->click_y)
                    {
                        ctx.fill({i * (CELL_SIZE + 1), j * (CELL_SIZE + 1), CELL_SIZE, CELL_SIZE}, CELL_MINE);
                    }
                    ctx.draw_image(duck, {i * (CELL_SIZE + 1) + (CELL_SIZE - IMAGE_SIZE) / 2, j * (CELL_SIZE + 1) + (CELL_SIZE - IMAGE_SIZE) / 2});
                }
                else
                {
                    ctx.fill({i * (CELL_SIZE + 1), j * (CELL_SIZE + 1), CELL_SIZE, CELL_SIZE}, CELL_ACTIVE);
                    Gfx::Rect rect = {i * (CELL_SIZE + 1), j * (CELL_SIZE + 1), CELL_SIZE, CELL_SIZE};
                    auto v = board.value(i, j);
                    if (v != 0)
                    {
                        ctx.draw_text(std::to_string(v).c_str(),
                                      rect,
                                      UI::TextAlignment::CENTER, UI::TextAlignment::CENTER, UI::Theme::font(),
                                      TEXT_COLOR);
                    }
                }
            }
        }
    }
}