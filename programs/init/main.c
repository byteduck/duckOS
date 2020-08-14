#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

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

// The init system for duckOS.

int main(int argc, char** argv) {
	if(getpid() != 1) {
		printf("pid != 1. Exiting.\n");
		return -1;
	}

	setsid();

	printf("[init] Welcome to duckOS!\n");
	while(1) {
		pid_t pid = waitpid(-1, NULL, 0);
		if(pid < 0 && errno == ECHILD) break; //All child processes exited
	}
	printf("[init] All child processes exited.\n");
}