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

#pragma once

class Process;
class Thread;
class Blocker {
public:
	virtual bool is_ready() = 0;
	virtual bool can_be_interrupted();
	virtual bool is_lock();
	virtual Thread* responsible_thread();

	void interrupt();
	void reset_interrupted();
	bool was_interrupted();

protected:
	virtual void on_interrupted();

private:
	bool _interrupted = false;
};

