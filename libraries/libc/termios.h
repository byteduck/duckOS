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

	Copyright (c) Byteduck 2016-2021. All rights reserved.
*/

#ifndef DUCKOS_LIBC_TERMIOS_H
#define DUCKOS_LIBC_TERMIOS_H

#include <sys/cdefs.h>
#include <sys/types.h>

__DECL_BEGIN

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

int tcgetattr(int fd, struct termios* termios_p);
int tcsetattr(int fd, int optional_actions, const struct termios* termios_p);
int tcflush(int fd, int queue_selector);
int tcflow(int fd, int action);
void cfmakeraw(struct termios* termios_p);
speed_t cfgetispeed(const struct termios* termios_p);
speed_t cfgetospeed(const struct termios* termios_p);
int cfsetispeed(struct termios* termios_p, speed_t speed);
int cfsetospeed(struct termios* termios_p, speed_t speed);

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

__DECL_END

#endif //DUCKOS_LIBC_TERMIOS_H
