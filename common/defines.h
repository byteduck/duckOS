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

#ifndef DUCKOS_DEFINES_H
#define DUCKOS_DEFINES_H

#define O_RDONLY  	0x000000
#define O_WRONLY  	0x000001
#define O_RDWR    	0x000002
#define O_APPEND  	0x000008
#define O_CREAT   	0x000200
#define O_TRUNC   	0x000400
#define O_EXCL    	0x000800
#define O_SYNC    	0x002000
#define O_NONBLOCK	0x004000
#define O_NOCTTY  	0x008000
#define O_CLOEXEC	0x040000
#define O_NOFOLLOW	0x100000
#define O_DIRECTORY 0x200000
#define O_EXEC		0x400000
#define O_SEARCH	O_EXEC //They're the same

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define SUCCESS 0
#define EPERM 1
#define ENOENT 2
#define ESRCH 3
#define EINTR 4
#define EIO 5
#define ENXIO 6
#define E2BIG 7
#define ENOEXEC 8
#define EBADF 9
#define ECHILD 10
#define EAGAIN 11
#define ENOMEM 12
#define EACCES 13
#define EFAULT 14
#define ENOTBLK 15
#define EBUSY 16
#define EEXIST 17
#define EXDEV 18
#define ENODEV 19
#define ENOTDIR 20
#define EISDIR 21
#define EINVAL 22
#define ENFILE 23
#define EMFILE 24
#define ENOTTY 25
#define ETXTBSY 26
#define EFBIG 27
#define ENOSPC 28
#define ESPIPE 29
#define EROFS 30
#define EMLINK 31
#define EPIPE 32
#define ERANGE 33
#define ENAMETOOLONG 34
#define ELOOP 35
#define EOVERFLOW 36
#define EOPNOTSUPP 37
#define ENOSYS 38
#define ENOTIMPL 39
#define EAFNOSUPPORT 40
#define ENOTSOCK 41
#define EADDRINUSE 42
#define EWHYTHO 43
#define ENOTEMPTY 44
#define EDOM 45
#define ECONNREFUSED 46
#define EADDRNOTAVAIL 47
#define EISCONN 48
#define ECONNABORTED 49
#define EALREADY 50
#define ECONNRESET 51
#define EDESTADDRREQ 52
#define EHOSTUNREACH 53
#define EILSEQ 54
#define EMSGSIZE 55
#define ENETDOWN 56
#define ENETUNREACH 57
#define ENETRESET 58
#define ENOBUFS 59
#define ENOLCK 60
#define ENOMSG 61
#define ENOPROTOOPT 62
#define ENOTCONN 63
#define EWOULDBLOCK 64
#define EPROTONOSUPPORT 65
#define EDEADLK 66
#define ETIMEDOUT 67
#define EPROTOTYPE 68
#define EINPROGRESS 69
#define ENOTHREAD 70
#define EPROTO 71
#define ENOTSUP 72
#define EMAXERRNO 73

#endif //DUCKOS_DEFINES_H
