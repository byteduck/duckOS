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

#include <kernel/filesystem/LinkedInode.h>
#include "ProcessArgs.h"

ProcessArgs::ProcessArgs(const kstd::shared_ptr<LinkedInode>& working_dir): working_dir(working_dir) {}

const void* ProcessArgs::setup_stack(void *stackptr) {
	auto*& stack8 = reinterpret_cast<uint8_t *&>(stackptr);
	auto*& stack32 = reinterpret_cast<uint32_t *&>(stackptr);

	auto* argp = new size_t[argv.size()];

	//Copy contents of all args into stack
	for(auto i = argv.size(); i > 0; i--) {
		stack8 -= argv[i - 1].length() + 1;
		argp[i - 1] = (size_t)stack8;
		strcpy((char*)stack8, argv[i - 1].c_str());
	}

	//Copy pointers to all args into stack
	for(auto i = argv.size(); i > 0; i--) {
		*--stack32 = argp[i - 1];
	}

	void* argvp = stackptr;

	//Push argc, argv, and env on to stack
	*--stack32 = 0; //env
	*--stack32 = (uint32_t) argvp; //argv
	*--stack32 = argv.size(); //argc

	return stackptr;
}
