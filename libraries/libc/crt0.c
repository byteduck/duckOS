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

int main(int, char**, char**);
void (*__init_array_start[])(int, char**, char**);
void (*__init_array_end[])(int, char**, char**);

int _start(int argc, char* argv[], char* env[]) {
	//environ = env;
	//_init();
	for (size_t i = 0; i < __init_array_end - __init_array_start; i++)
		(*__init_array_start[i])(argc, argv, env);

	int ret = main(argc, argv, env);
	exit(ret);
	return -1;
}