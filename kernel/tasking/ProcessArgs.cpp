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

#include "ProcessArgs.h"
#include <kernel/filesystem/LinkedInode.h>
#include <kernel/kstd/cstring.h>
#include <kernel/memory/Stack.h>

ProcessArgs::ProcessArgs(const kstd::shared_ptr<LinkedInode>& working_dir): working_dir(working_dir) {}

void ProcessArgs::setup_stack(Stack& stack) {
	auto* argp = new size_t[argv.size()];
	auto* envp = new size_t[env.size()];

	//Copy contents of all args into stack
	for(auto i = argv.size(); i > 0; i--) {
		stack.push_string(argv[i - 1].c_str());
		argp[i - 1] = stack.real_stackptr();
	}

	//Copy contents of all env into stack
	for(auto i = env.size(); i > 0; i--) {
		stack.push_string(env[i - 1].c_str());
		envp[i - 1] = stack.real_stackptr();
	}

	//Copy pointers to all args into stack
	stack.push32(0);
	for(auto i = argv.size(); i > 0; i--)
		stack.push32(argp[i - 1]);
	size_t argvp = stack.real_stackptr();

	//Copy pointers to all env into stack
	stack.push32(0);
	for(auto i = env.size(); i > 0; i--)
		stack.push32(envp[i - 1]);
	size_t envpp = stack.real_stackptr();

	//Push argc, argv, and env on to stack
	stack.push_sizet(envpp); //env
	stack.push_sizet(argvp); //argv
	stack.push_int(argv.size()); //argc
	stack.push32(0);
}
