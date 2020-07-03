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

#ifndef DUCKOS_PROCESSARGS_H
#define DUCKOS_PROCESSARGS_H


#include <common/vector.hpp>

class ProcessArgs {
public:
	ProcessArgs(const DC::shared_ptr<LinkedInode>& working_dir);

	const void* setup_stack(void* stackptr);

	DC::vector<DC::string> argv;
	DC::vector<DC::string> env;
	DC::shared_ptr<LinkedInode> working_dir;
};


#endif //DUCKOS_PROCESSARGS_H
