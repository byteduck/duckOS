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

#include "termios.h"
#include "sys/ioctl.h"
#include "errno.h"

int tcgetattr(int fd, struct termios* termios_p) {
	return ioctl(fd, TCGETS, termios_p);
}

int tcsetattr(int fd, int optional_actions, const struct termios* termios_p) {
	switch(optional_actions) {
		case TCSANOW:
			return ioctl(fd, TCSETS, termios_p);
		case TCSADRAIN:
			return ioctl(fd, TCSETSW, termios_p);
		case TCSAFLUSH:
			return ioctl(fd, TCSETSF, termios_p);
		default:
			errno = EINVAL;
			return -1;
	}
}

int tcflush(int fd, int queue_selector) {
	return ioctl(fd, TCFLSH, queue_selector);
}

int tcflow(int fd, int action) {
	errno = ENOSYS;
	return -1;
}

void cfmakeraw(struct termios* termios_p) {
	termios_p->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	termios_p->c_oflag &= ~OPOST;
	termios_p->c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	termios_p->c_cflag &= ~(CSIZE | PARENB);
	termios_p->c_cflag |= CS8;
}

speed_t cfgetispeed(const struct termios* termios_p) {
	return termios_p->c_ispeed;
}

speed_t cfgetospeed(const struct termios* termios_p) {
	return termios_p->c_ospeed;
}

int cfsetispeed(struct termios* termios_p, speed_t speed) {
	termios_p->c_ispeed = speed;
	errno = ENOSYS;
	return -1;
}

int cfsetospeed(struct termios* termios_p, speed_t speed) {
	termios_p->c_ospeed = speed;
	errno = ENOSYS;
	return -1;
}