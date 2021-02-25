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

#ifndef DUCKOS_4INAROW_MAIN_H
#define DUCKOS_4INAROW_MAIN_H

#define ROWS 6
#define COLUMNS 7
#define INAROW 4
#define MAX_DEPTH 5

struct MoveScore {
	int num_moves = 0;
	int num_win = 0;
	int num_lose = 0;
} __attribute__((aligned(16)));

typedef int Board[ROWS][COLUMNS];

void reset_board(Board board);
void print_board(Board board);
bool is_valid_move(Board board, int move);
int player_pick_move(Board board, int player);
MoveScore score_move(Board board, int move, int player, int cur_player, int depth);
int computer_pick_move(Board board, int player);
bool add_piece(Board board, int move, int player);
bool check_win(Board board);

#endif //DUCKOS_4INAROW_MAIN_H
