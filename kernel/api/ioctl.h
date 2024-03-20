/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

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

#define SIOCSIFADDR 14
#define SIOCSIFNETMASK 15
#define SIOCADDRT 16