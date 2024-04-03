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

#ifndef DUCKOS_LIBC_SIGNAL_H
#define DUCKOS_LIBC_SIGNAL_H

#include "sys/cdefs.h"
#include "stdint.h"
#include "sys/types.h"

__DECL_BEGIN

#include <kernel/api/signal.h>

void (*signal(int sig, sighandler_t func))(int);
int raise(int sig);
int kill(pid_t pid, int sig);
void sigaction(int signum, const struct sigaction* act, const struct sigaction* oldact);
int sigemptyset(sigset_t* set);
int sigfillset(sigset_t* set);
int sigaddset(sigset_t* set, int sig);
int sigdelset(sigset_t* set, int sig);

__DECL_END

#endif //DUCKOS_LIBC_SIGNAL_H
