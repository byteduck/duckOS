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

#include "4inarow.h"
#include <stdio.h>
#include <algorithm>

void reset_board(Board board) {
	for(int i = 0; i < ROWS; i++) {
		for(int j = 0; j < COLUMNS; j++) {
			board[i][j] = 0;
		}
	}
}

void print_board(Board board) {
	for(int i = 0; i < ROWS; i++) {
		for(int j = 0; j < COLUMNS; j++) {
			auto& space = board[i][j];
			printf("%c ", space == 0 ? ' ' : space == 1 ? 'X' : 'O');
		}
		printf("\n");
	}

	for(int i = 0; i < COLUMNS; i++) {
		printf("%d ", i + 1);
	}
	printf("\n");
}

bool is_valid_move(Board board, int move) {
	return move >= 0 && move < COLUMNS && board[0][move] == 0;
}

int player_pick_move(Board board, int player) {
	int move;
	do {
		printf("Player %d pick a move: ", player);
		fflush(stdout);
		scanf("%d", &move);
		move--;
	} while(!is_valid_move(board, move));
	return move;
}

MoveScore score_move(Board board, int move, int player, int cur_player, int depth) {
	MoveScore ret {0, 0, 0};
	if(is_valid_move(board, move)) {
		ret.num_moves++;
		Board new_board;
		std::copy(&board[0][0], &board[0][0]+ROWS*COLUMNS,&new_board[0][0]);
		add_piece(new_board, move, cur_player);
		if(check_win(new_board)) {
			if(cur_player == player)
				ret.num_win++;
			else
				ret.num_lose++;
		} else {
			if(depth < MAX_DEPTH) {
				for(int i = 0; i < COLUMNS; i++) {
					if(is_valid_move(board, i)) {
						auto prob = score_move(new_board, i, player, cur_player == 1 ? 2 : 1, depth + 1);
						ret.num_win += prob.num_win;
						ret.num_lose += prob.num_lose;
						ret.num_moves += prob.num_moves;
					}
				}
			}
		}
	}
	return ret;
}

int computer_pick_move(Board board, int player) {
	int best_move = 0;
	double best_prob = -1.0;
	for(int i = 0; i < COLUMNS; i++) {
		MoveScore prob = score_move(board, i, player, player, 0);
		double p = ((double) prob.num_win / prob.num_moves) - ((double) prob.num_lose / prob.num_moves);
		if(p > best_prob) {
			best_move = i;
			best_prob = p;
		}
	}
	return best_move;
}

bool add_piece(Board board, int move, int player) {
	if(!is_valid_move(board, move))
		return false;

	for(int i = ROWS - 1; i >= 0; i--) {
		if(board[i][move] == 0) {
			board[i][move] = player;
			return true;
		}
	}

	return false;
}

bool check_win(Board board) {
	for(int i = 0; i < ROWS; i++) {
		for(int j = 0; j < COLUMNS; j++) {
			int firstpiece = board[i][j];
			bool not_win = false;

			if(firstpiece == 0)
				continue;

			//Check vertical
			if(i >= INAROW - 1) {
				for(int row = 0; row < INAROW; row++) {
					if(board[i - row][j] != firstpiece) {
						not_win = true;
						break;
					}
				}
				if(!not_win)
					return true;
			}

			//Check horizontal
			if(j <= COLUMNS - INAROW) {
				not_win = false;
				for(int col = 0; col < INAROW; col++) {
					if(board[i][j + col] != firstpiece) {
						not_win = true;
						break;
					}
				}
				if(!not_win)
					return true;
			}

			//Check diagonal up & to right
			if(i >= INAROW - 1 && j <= COLUMNS - INAROW) {
				not_win = false;
				for(int space = 0; space < INAROW; space++) {
					if(board[i - space][j + space] != firstpiece) {
						not_win = true;
						break;
					}
				}
				if(!not_win)
					return true;
			}

			//Check diagonal down & to right
			if(i <= ROWS - INAROW && j <= COLUMNS - INAROW) {
				not_win = false;
				for(int space = 0; space < INAROW; space++) {
					if(board[i + space][j + space] != firstpiece) {
						not_win = true;
						break;
					}
				}
				if(!not_win)
					return true;
			}
		}
	}

	return false;
}