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

#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>

int main(int argc, char **argv, char **env) {
	DIR *dir;
	struct dirent *ent;
	char* dirname = ".";
	if(argc >= 2) {
		dirname = argv[1];
	}

	if ((dir = opendir (dirname)) != NULL) {
		while ((ent = readdir (dir)) != NULL) {
			switch(ent->d_type) {
				case DT_REG:
					printf("[REG] ");
					break;
				case DT_DIR:
					printf("[DIR] ");
					break;
				case DT_BLK:
					printf("[BLK] ");
					break;
				case DT_CHR:
					printf("[CHR] ");
					break;
				case DT_FIFO:
					printf("[FIFO] ");
					break;
				case DT_LNK:
					printf("[LNK] ");
					break;
				case DT_SOCK:
					printf("[SOCK] ");
					break;
				case DT_UNKNOWN:
					printf("[UNKNOWN] ");
					break;
				default:
					printf("[???] ");
					break;
			}
			printf ("%s\n", ent->d_name);
		}
		closedir (dir);
	} else {
		printf("Cannot mkdir '%s': %s\n", argv[1], strerror(errno));
		return errno;
	}
}