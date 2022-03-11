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
    
    Copyright (c) Byteduck 2016-2022. All rights reserved.
*/

#include "Uxn.h"
#include <libduck/Args.h>
#include <libduck/Stream.h>

Duck::Args args;
std::string filename;
int main(int argc, char** argv, char** envp) {
	args.add_positional(filename, true, "ROMFILE", "The ROM file to load.");
	args.parse(argc, argv);

	Uxn machine;
	auto res = machine.load_rom(filename);
	if(res.is_error())
		Duck::Stream::std_err << "Couldn't load rom " << filename << ": " << res.message() << "\n";
	while(machine.step()) {}
	fflush(stdout);
	return 0;
}