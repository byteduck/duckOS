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

	Copyright (c) ChazizGRKB 2022.
*/

#include <stdio.h>
#include <libduck/FileStream.h>
#include "libsys/Memory.h"

using namespace Sys;

int main(int argc, char** argv) {

	//TODO: CPU string

	// Operating system version number
	auto version_file = Duck::FileInputStream("/etc/ver");
	std::string ver;
	version_file >> ver;

	// Memory
	Sys::Mem::Info m_mem_info = Mem::get_info();

	printf("OS: duckOS %s\n", ver.c_str());
	printf("Memory: %s\n", m_mem_info.usable.readable().c_str());
}