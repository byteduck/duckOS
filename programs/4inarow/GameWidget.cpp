/*
    This file is part of duckOS.

    duckOS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    duckOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with duckOS.  If not, see <https://www.gnu.org/licenses/>.

    Copyright (c) Byteduck 2016-2020. All rights reserved.
*/

#include "GameWidget.h"
#include <libgraphics/font.h>
#include <libui/libui.h>

GameWidget::GameWidget() {
	reset_board(board);
}

void GameWidget::reset(bool vs_cpu) {
	reset_board(board);
	was_win = false;
	current_player = 1;
	status = "Player 1 choose";
	hovered_cell = {-1, -1};
	player2_computer = vs_cpu;
	repaint();
}

void GameWidget::show_hint() {
	int col = computer_pick_move(board, current_player);
	int row = ROWS;
	for(int i = ROWS - 1; i >= 0; i--) {
		if(board[i][col] == 0) {
			row = i;
			break;
		}
	}
	hint_cell = {col, row};
	repaint();
}

Dimensions GameWidget::preferred_size() {
	return {COLUMNS * CELL_SIZE + 4, ROWS * CELL_SIZE + 17};
}

void GameWidget::do_repaint(const UI::DrawContext& ctx) {
	ctx.fill({0, 0, ctx.width(), ctx.height()}, CELL_COLOR);
	ctx.draw_inset_rect({0, 0, ctx.width(), ROWS * CELL_SIZE + 4}, CELL_COLOR);
	for(int row = 0; row < ROWS; row++) {
		for(int col = 0; col < COLUMNS; col++) {
			uint32_t color;
			if(!was_win && (hovered_cell == Point {col, row} || hint_cell == Point {col, row})) {
				color = current_player == 1 ? HOVER1_COLOR : HOVER2_COLOR;
			} else {
				switch(board[row][col]) {
					case 0:
						color = EMPTY_COLOR;
						break;
					case 1:
						color = PLAYER1_COLOR;
						break;
					case 2:
						color = PLAYER2_COLOR;
						break;
				}
			}
			ctx.fill({
					col * CELL_SIZE + 4, row * CELL_SIZE + 4, CELL_SIZE - 4, CELL_SIZE - 4
				}, color);
		}
	}
	ctx.draw_text(status.c_str(), {2, ROWS * CELL_SIZE + 6}, UI::Theme::font(), RGB(255, 255, 255));
}

bool GameWidget::on_mouse_move(Pond::MouseMoveEvent evt) {
	int new_hovered_cell = evt.new_pos.x / CELL_SIZE;
	if(new_hovered_cell != hovered_cell.x) {
		int row = ROWS;
		for(int i = ROWS - 1; i >= 0; i--) {
			if(board[i][new_hovered_cell] == 0) {
				row = i;
				break;
			}
		}
		hovered_cell = {new_hovered_cell, row};
		repaint();
	}
	return true;
}

bool GameWidget::on_mouse_button(Pond::MouseButtonEvent evt) {
	if((evt.new_buttons & POND_MOUSE1) && !(evt.old_buttons & POND_MOUSE1)) {
		if(!was_win && is_valid_move(board, hovered_cell.x)) {
			add_piece(board, hovered_cell.x, current_player);
			hovered_cell = {-1, -1};
			hint_cell = {-1, -1};

			std::string player_text = current_player == 1 ? "1" : "2";

			if(check_win(board)) {
				status = "Player " + player_text + " wins!";
				was_win = true;
			} else {
				if(player2_computer) {
					add_piece(board, computer_pick_move(board, 2), 2);
					if(check_win(board)) {
						status = "Computer wins!";
						was_win = true;
					}
				} else {
					current_player = current_player == 1 ? 2 : 1;
					player_text = current_player == 1 ? "1" : "2";
					status = "Player " + player_text + " choose";
				}
			}
			repaint();
		} else {
			reset(player2_computer);
		}
		return true;
	}
	return false;
}
