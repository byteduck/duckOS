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

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/syscall.h>

struct __attribute__((packed)) krnl_dirent {
	ino_t inode;
	uint8_t mode;
	size_t namelen;
	char name[];
};

DIR *opendir(const char *name) {
	int fd = open(name, O_RDONLY | O_DIRECTORY);
	if (fd == -1)
		return 0;
	DIR* dirp = (DIR*) malloc(sizeof(DIR));
	dirp->dd_fd = fd;
	dirp->dd_seek = 0;
	dirp->dd_buf = 0;
	dirp->dd_len = 0;
	return dirp;
}

int __read_dir(int file, char *ptr, size_t len) {
	int ret = 0;
	asm volatile("int $0x80" :: "a"(SYS_READDIR), "b"(file), "c"(ptr), "d"(len));
	asm volatile("mov %%eax, %0" : "=r"(ret));
	return ret;
}

struct dirent *readdir(DIR *dirp) {
	//If the buffer hasn't been initialized yet, initialize and read
	if(!dirp->dd_buf) {
		struct stat st;
		fstat(dirp->dd_fd, &st);
		size_t dirsize = st.st_size;
		dirp->dd_buf = (char*)malloc(dirsize);
		ssize_t res = __read_dir(dirp->dd_fd, dirp->dd_buf, dirsize);
		dirp->dd_len = res;
		if(res < 0) {
			errno = -res;
			free(dirp->dd_buf);
			dirp->dd_buf = 0;
			return 0;
		}
	}

	//If the next entry would be out of the bounds of the buffer, return 0
	if(dirp->dd_seek + sizeof(struct krnl_dirent) >= dirp->dd_len) return 0;

	struct krnl_dirent* kent = (struct krnl_dirent*)(dirp->dd_buf + dirp->dd_seek);
	unsigned short reclen = sizeof(struct krnl_dirent) + sizeof(char) * kent->namelen;

	//If the reclen is out of bounds of the buffer, return 0
	if(reclen + dirp->dd_seek > dirp->dd_len) return 0;

	struct dirent* dent = &dirp->dd_cur;

	//Copy information from krnl_dirent into dirent
	dent->d_reclen = reclen;
	for (size_t i = 0; i < kent->namelen; i++)
		dent->d_name[i] = kent->name[i];
	dent->d_name[kent->namelen] = '\0';
	dent->d_ino = kent->inode;
	dent->d_type = kent->mode;
	dent->d_off = 0;

	//Adjust seek accordingly
	dirp->dd_seek += reclen;

	return dent;
}

int readdir_r(DIR *__restrict dirp, struct dirent *__restrict entry,
			  struct dirent **__restrict result) {
	return -1; //NOT IMPLEMENTED
}

void rewinddir(DIR *dirp) {
	dirp->dd_seek = 0;
}

int closedir(DIR *dirp) {
	if (!dirp || dirp->dd_fd == -1)
		return -EBADF;
	if (dirp->dd_buf)
		free(dirp->dd_buf);
	int rc = close(dirp->dd_fd);
	if (rc == 0)
		dirp->dd_fd = -1;
	free(dirp);
	return rc;
}