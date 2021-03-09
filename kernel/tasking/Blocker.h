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

#ifndef DUCKOS_BLOCKER_H
#define DUCKOS_BLOCKER_H

class Process;
class Blocker {
public:
	~Blocker();
	virtual bool is_ready() = 0;

	void unblock();

protected:
	friend class Process;
	void assign_process(Process* proc);
	void clear_process();

private:
	Process* _proc = nullptr;
};

#endif //DUCKOS_BLOCKER_H
