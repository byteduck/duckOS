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

	Copyright (c) 2023. All rights reserved.
*/
#pragma once

#include <stdlib.h>
#include <libduck/Log.h>
#include <ctime>

#define COLUMNS 16
#define ROWS 16

#define SHOW 1
#define HIDE 0
#define EMPTY 0
#define MARKED 32
#define MINE 64
#define HIDDEN 128
#define NUM_MINES 40

typedef uint8_t Cell;

class Board {

public:
	void reset_board();
	void reveil_cell(int x, int y);
	void mark_cell(int x, int y);
	bool is_visible(int x, int y) const;
	bool is_marked(int x, int y) const;
	bool is_mine(int x, int y) const;
	bool can_move() const { return m_can_move; }
	int value(int x, int y) const;
	int columns() { return COLUMNS; }
	int rows() { return ROWS; }
private:
	int sourunding_mines(int x, int y);
	void reveil_cell_recursive(int x, int y);
	void show_all_mines();
	void stop();
	bool all_mines_marked() const;
	Cell board[ROWS][COLUMNS];
	Cell marked[ROWS][COLUMNS];
	Cell visible[ROWS][COLUMNS];
	bool m_can_move;
};