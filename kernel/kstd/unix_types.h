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

#ifndef DUCKOS_UNIX_TYPES_H
#define DUCKOS_UNIX_TYPES_H

#include "types.h"

/// Misc typedefs
typedef int pid_t;
typedef int64_t time_t;
struct timespec {
	time_t tv_sec;
	long tv_usec;
};
typedef void (*sighandler_t)(int);
typedef unsigned long sigset_t;
typedef struct sigaction {
	sighandler_t sa_sigaction;
	sigset_t sa_mask;
	int sa_flags;
} sigaction_t;

/// IO
typedef unsigned long ino_t;
typedef short dev_t;
typedef uint32_t mode_t;
typedef unsigned short nlink_t;
typedef unsigned short uid_t;
typedef unsigned short gid_t;
typedef long off_t;
typedef long blksize_t;
typedef long blkcnt_t;

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
#define	EPERM 1		/* Not owner */
#define	ENOENT 2	/* No such file or directory */
#define	ESRCH 3		/* No such process */
#define	EINTR 4		/* Interrupted system call */
#define	EIO 5		/* I/O error */
#define	ENXIO 6		/* No such device or address */
#define	E2BIG 7		/* Arg list too long */
#define	ENOEXEC 8	/* Exec format error */
#define	EBADF 9		/* Bad file number */
#define	ECHILD 10	/* No children */
#define	EAGAIN 11	/* No more processes */
#define	ENOMEM 12	/* Not enough space */
#define	EACCES 13	/* Permission denied */
#define	EFAULT 14	/* Bad address */
#define	EBUSY 16	/* Device or resource busy */
#define	EEXIST 17	/* File exists */
#define	EXDEV 18	/* Cross-device link */
#define	ENODEV 19	/* No such device */
#define	ENOTDIR 20	/* Not a directory */
#define	EISDIR 21	/* Is a directory */
#define	EINVAL 22	/* Invalid argument */
#define	ENFILE 23	/* Too many open files in system */
#define	EMFILE 24	/* File descriptor value too large */
#define	ENOTTY 25	/* Not a character device */
#define	ETXTBSY 26	/* Text file busy */
#define	EFBIG 27	/* File too large */
#define	ENOSPC 28	/* No space left on device */
#define	ESPIPE 29	/* Illegal seek */
#define	EROFS 30	/* Read-only file system */
#define	EMLINK 31	/* Too many links */
#define	EPIPE 32	/* Broken pipe */
#define	EDOM 33		/* Mathematics argument out of domain of function */
#define	ERANGE 34	/* Result too large */
#define	ENOMSG 35	/* No message of desired type */
#define	EIDRM 36	/* Identifier removed */
#define	EDEADLK 45	/* Deadlock */
#define	ENOLCK 46	/* No lock */
#define ENOSTR 60	/* Not a stream */
#define ENODATA 61	/* No data (for no delay io) */
#define ETIME 62	/* Stream ioctl timeout */
#define ENOSR 63	/* No stream resources */
#define ENOLINK 67	/* Virtual circuit is gone */
#define EPROTO 71	/* Protocol error */
#define	EMULTIHOP 74	/* Multihop attempted */
#define EBADMSG 77	/* Bad message */
#define EFTYPE 79	/* Inappropriate file type or format */
#define ENOSYS 88	/* Function not implemented */
#define ENOTEMPTY 90	/* Directory not empty */
#define ENAMETOOLONG 91	/* File or path name too long */
#define ELOOP 92	/* Too many symbolic links */
#define EOPNOTSUPP 95	/* Operation not supported on socket */
#define EPFNOSUPPORT 96 /* Protocol family not supported */
#define ECONNRESET 104  /* Connection reset by peer */
#define ENOBUFS 105	/* No buffer space available */
#define EAFNOSUPPORT 106 /* Address family not supported by protocol family */
#define EPROTOTYPE 107	/* Protocol wrong type for socket */
#define ENOTSOCK 108	/* Socket operation on non-socket */
#define ENOPROTOOPT 109	/* Protocol not available */
#define ECONNREFUSED 111	/* Connection refused */
#define EADDRINUSE 112		/* Address already in use */
#define ECONNABORTED 113	/* Software caused connection abort */
#define ENETUNREACH 114		/* Network is unreachable */
#define ENETDOWN 115		/* Network interface is not configured */
#define ETIMEDOUT 116		/* Connection timed out */
#define EHOSTDOWN 117		/* Host is down */
#define EHOSTUNREACH 118	/* Host is unreachable */
#define EINPROGRESS 119		/* Connection already in progress */
#define EALREADY 120		/* Socket already connected */
#define EDESTADDRREQ 121	/* Destination address required */
#define EMSGSIZE 122		/* Message too long */
#define EPROTONOSUPPORT 123	/* Unknown protocol */
#define EADDRNOTAVAIL 125	/* Address not available */
#define ENETRESET 126		/* Connection aborted by network */
#define EISCONN 127		/* Socket is already connected */
#define ENOTCONN 128		/* Socket is not connected */
#define ETOOMANYREFS 129
#define EDQUOT 132
#define ESTALE 133
#define ENOTSUP 134		/* Not supported */
#define EILSEQ 138		/* Illegal byte sequence */
#define EOVERFLOW 139	/* Value too large for defined data type */
#define ECANCELED 140	/* Operation canceled */
#define ENOTRECOVERABLE 141	/* State not recoverable */
#define EOWNERDEAD 142	/* Previous owner died */
#define EWOULDBLOCK EAGAIN	/* Operation would block */

/// ioctl
#define TIOCSCTTY 	1
#define TIOCGPGRP	2
#define TIOCSPGRP	3
#define TCGETS		4
#define TCSETS		5
#define TCSETSW		6
#define TCSETSF		7
#define TCFLSH		8
#define TIOCSWINSZ	9
#define TIOCGWINSZ	10
#define TIOCNOTTY	11
#define TIOSGFX		12
#define TIOSNOGFX	13

/// termios
#define NCCS 32

typedef uint32_t speed_t;
typedef uint32_t tcflag_t;
typedef uint8_t cc_t;

struct termios {
	tcflag_t c_iflag;
	tcflag_t c_oflag;
	tcflag_t c_cflag;
	tcflag_t c_lflag;
	cc_t c_cc[NCCS];
	tcflag_t c_ispeed;
	tcflag_t c_ospeed;
};

struct winsize {
	unsigned short ws_row;
	unsigned short ws_col;
	unsigned short ws_xpixel;
	unsigned short ws_ypixel;
};

//c_iflag
#define IGNBRK	0x1
#define BRKINT	0x2
#define IGNPAR	0x4
#define PARMRK	0x8
#define INPCK	0x10
#define ISTRIP	0x20
#define INLCR	0x40
#define IGNCR	0x80
#define ICRNL	0x100
#define IUCLC	0x200
#define IXON	0x400
#define IXANY	0x800
#define IXOFF	0x1000
#define IMAXBEL	0x2000
#define IUTF8	0x4000

//c_oflag
#define OPOST	0x1
#define OLCUC	0x2
#define ONLCR	0x4
#define OCRNL	0x8
#define ONOCR	0x10
#define ONLRET	0x20
#define OFILL	0x40
#define OFDEL	0x80
#define NLDLY	0x100
#define CRDLY	0x200
#define TABDLY	0x400
#define BSDLY	0x800
#define VTDLY	0x1000
#define FFDLY	0x2000

//c_cflag
#define CSIZE	0x6
#define CS5		0x0
#define CS6		0x2
#define CS7		0x4
#define CS8		0x6
#define CSTOPB	0x8
#define CREAD	0x10
#define PARENB	0x20
#define PARODD	0x40
#define HUPCL	0x80
#define CLOCAL	0x100

//c_lflag
#define ISIG	0x1
#define ICANON	0x2
#define XCASE	0x4
#define ECHO	0x8
#define ECHOE	0x10
#define EHCOK	0x20
#define ECHONL	0x40
#define ECHOCTL	0x80
#define ECHOPRT	0x100
#define ECHOKE	0x200
#define DEFECHO	0x400
#define FLUSHO	0x800
#define NOFLSH	0x1000
#define TOSTOP	0x2000
#define PENDIN	0x4000
#define IEXTEN	0x8000

//c_cc
#define VDISCARD	0
#define VDSUSP		1
#define VEOF		2
#define VEOL		3
#define VEOL2		4
#define VERASE		5
#define VINTR		6
#define VKILL		7
#define VLNEXT		8
#define VMIN		9
#define VQUIT		10
#define VREPRINT	11
#define VSTART		12
#define VSTATUS		13
#define VSTOP		14
#define VSUSP		15
#define VSWTCH		16
#define VTIME		17
#define VWERASE		18

//tcsetattr optional actions
#define TCSANOW		1
#define TCSADRAIN	2
#define TCSAFLUSH	3

//tcflow actions
#define TCOOFF	0
#define TCOON	1
#define TCIOFF	2
#define TCION	3

//tcflush queue selector
#define TCIFLUSH	0
#define TCOFLUSH	1
#define TCIOFLUSH	2

#endif //DUCKOS_UNIX_TYPES_H
