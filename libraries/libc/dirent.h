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

#ifndef DUCKOS_LIBC_DIRENT_H
#define DUCKOS_LIBC_DIRENT_H

#include <sys/types.h>

__DECL_BEGIN

#define MAXNAMELEN 256		/* sizeof(struct dirent.d_name)-1 */

#define DT_UNKNOWN 0
#define DT_REG 1
#define DT_DIR 2
#define DT_CHR 3
#define DT_BLK 4
#define DT_FIFO 5
#define DT_SOCK 6
#define DT_LNK 7

struct dirent {
	ino_t d_ino;
	off_t d_off;
	unsigned short int d_reclen;
	unsigned char d_type;
	char d_name[MAXNAMELEN];
};

typedef struct {
	int dd_fd;		      /* directory file */
	int dd_seek;          /* seek in file */
	char *dd_buf;	      /* buffer */
	int dd_len;		      /* buffer length */
	struct dirent dd_cur; /* current entry */
} DIR;

DIR* opendir(const char *dirname);
struct dirent* readdir(DIR *dirname);
int readdir_r(DIR* dirp, struct dirent** entry, struct dirent** result);
void rewinddir(DIR* dirp);
int closedir(DIR* dirp);

__DECL_END

#endif //DUCKOS_LIBC_DIRENT_H
