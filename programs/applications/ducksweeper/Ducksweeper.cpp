#include "Ducksweeper.h"

bool is_empty(Cell cell)
{
	return !(cell & MINE == MINE);
}

void Board::reset_board()
{
	srand(time(nullptr));
	for (int i = 0; i < COLUMNS; i++)
	{
		for (int j = 0; j < ROWS; j++)
		{
			this->visible[i][j] = HIDE;
			this->board[i][j] = EMPTY;
			this->marked[i][j] = EMPTY;
		}
	}
	int generated_mines = 0;
	while (generated_mines <= NUM_MINES)
	{
		int x = rand() % COLUMNS;
		int y = rand() % ROWS;
		if (is_empty(board[x][y]))
		{
			generated_mines += 1;
			this->board[x][y] = MINE;
		}
	}
	for (int i = 0; i < COLUMNS; i++)
	{
		for (int j = 0; j < ROWS; j++)
		{
			if (this->board[i][j] != MINE)
			{
				this->board[i][j] = this->sourunding_mines(i, j);
			}
		}
	}
	this->m_can_move = true;
}

int Board::sourunding_mines(int x, int y)
{
	int mines = 0;
	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			int nx = x + j;
			int ny = y + i;
			if (nx < 0 || nx >= COLUMNS)
				continue;
			if (ny < 0 || ny >= ROWS)
				continue;

			if (this->board[nx][ny] == MINE)
			{
				mines += 1;
			}
		}
	}
	return mines;
}

void Board::reveil_cell(int x, int y)
{
	if (!this->can_move())
		return;
	if (x >= 0 && y >= 0 && x < COLUMNS && y < ROWS)
	{
		if (this->is_marked(x, y))
			return;
		this->visible[x][y] = SHOW;
		if (this->is_mine(x, y))
		{
			this->stop();
			this->show_all_mines();
		}
		if (this->board[x][y] == 0)
		{
			this->reveil_cell_recursive(x, y);
		}
	}
}

bool Board::all_mines_marked() const
{
	for (int x = 0; x < COLUMNS; x++)
	{
		for (int y = 0; y < ROWS; y++)
		{
			if ((!this->is_mine(x, y) && this->is_marked(x, y)) ||
				(this->is_mine(x, y) && !this->is_marked(x, y)))
			{
				return false;
			}
		}
	}
	return true;
}

void Board::stop()
{
	this->m_can_move = false;
}

void Board::show_all_mines()
{
	for (int x = 0; x < COLUMNS; x++)
	{
		for (int y = 0; y < ROWS; y++)
		{
			if (this->is_mine(x, y))
			{
				this->visible[x][y] = SHOW;
			}
		}
	}
}

void Board::reveil_cell_recursive(int x, int y)
{
	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			int dx = x + j;
			int dy = y + i;
			if (!this->is_visible(dx, dy) &&
				this->board[dx][dy] != MINE)
			{
				this->reveil_cell(dx, dy);
			}
		}
	}
}

void Board::mark_cell(int x, int y)
{
	if (!this->can_move())
		return;
	if (x >= 0 && y >= 0 && x < COLUMNS && y < ROWS)
	{
		if (this->is_visible(x, y) && !this->is_marked(x, y))
			return;
		this->marked[x][y] ^= 1;
		this->visible[x][y] ^= 1;
		if (this->all_mines_marked())
		{
			this->stop();
		}
	}
}

int Board::value(int x, int y) const
{
	if (x >= 0 && y >= 0 && x < COLUMNS && y < ROWS)
	{
		return (int)(this->board[x][y]);
	}
	return -1;
}

bool Board::is_visible(int x, int y) const
{
	return this->visible[x][y] == 1;
}

bool Board::is_marked(int x, int y) const
{
	return this->marked[x][y] == 1;
}

bool Board::is_mine(int x, int y) const
{
	return (this->board[x][y] & MINE) == MINE;
}