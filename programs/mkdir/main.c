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

//A simple program that prompts you to type something and repeats it back to you.

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

int main(int argc, char** argv) {
	if(argc < 2) {
		printf("Missing name operand\n");
		return 1;
	}
	int res = mkdir(argv[1], 0700);
	if(res != -1) return 0;
	switch(errno) {
		case ENOENT:
			printf("Cannot mkdir '%s': No such file or directory\n", argv[1]);
			break;
		case ENOTDIR:
			printf("Cannot mkdir '%s': Path is not a directory\n", argv[1]);
			break;
		case EEXIST:
			printf("Cannot mkdir '%s': Directory exists\n", argv[1]);
			break;
		default:
			printf("Canot mkdir '%s': Error %d\n", argv[1], errno);
	}
	return errno;
}
