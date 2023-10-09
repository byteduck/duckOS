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

#include "Signal.h"
const char* Signal::signal_names[] = {
		"0",
		"SIGHUP",
		"SIGINT",
		"SIGQUIT",
		"SIGILL",
		"SIGTRAP",
		"SIGABRT",
		"SIGEMT",
		"SIGFPE",
		"SIGKILL",
		"SIGBUS",
		"SIGSEGV",
		"SIGSYS",
		"SIGPIPE",
		"SIGALRM",
		"SIGTERM",
		"SIGURG",
		"SIGSTOP",
		"SIGTSTP",
		"SIGCONT",
		"SIGCHLD",
		"SIGTTIN",
		"SIGTTOU",
		"SIGIO",
		"SIGXCPU",
		"SIGXFSZ",
		"SIGVTALRM",
		"SIGPROF",
		"SIGWINCH",
		"SIGLOST",
		"SIGUSR1",
		"SIGUSR2",
		"NSIG"
};

Signal::SignalSeverity Signal::signal_severities[] = {
		NOKILL, //0
		KILL, //SIGHUP
		KILL, //SIGINT
		FATAL, //SIGQUIT
		FATAL, //SIGILL
		FATAL, //SIGTRAP
		FATAL, //SIGABRT
		FATAL, //SIGEMT
		FATAL, //SIGFPE
		KILL, //SIGKILL
		FATAL, //SIGBUS
		FATAL, //SIGSEGV
		FATAL, //SIGSYS
		NOKILL, //SIGPIPE
		NOKILL, //SIGALRM
		KILL, //SIGTERM
		NOKILL, //SIGURG
		NOKILL, //SIGSTOP
		NOKILL, //SIGTSTP
		NOKILL, //SIGCONT
		NOKILL, //SIGCHLD
		NOKILL, //SIGTTIN
		NOKILL, //SIGTTOU
		NOKILL, //SIGIO
		FATAL, //SIGXCPU
		FATAL, //SIGXFSZ
		NOKILL, //SIGVTALRM
		NOKILL, //SIGPROF
		NOKILL, //SIGWINCH
		FATAL, //SIGLOST
		NOKILL, //SIGUSR1
		NOKILL, //SIGUSR2
		NOKILL, //NSIG
};

bool Signal::dispatch_to_every_thread[] = {
		false, //0
		false, //SIGHUP
		false, //SIGINT
		false, //SIGQUIT
		false, //SIGILL
		false, //SIGTRAP
		false, //SIGABRT
		false, //SIGEMT
		false, //SIGFPE
		false, //SIGKILL
		false, //SIGBUS
		false, //SIGSEGV
		false, //SIGSYS
		false, //SIGPIPE
		false, //SIGALRM
		false, //SIGTERM
		false, //SIGURG
		true,  //SIGSTOP
		true,  //SIGTSTP
		true,  //SIGCONT
		false, //SIGCHLD
		false, //SIGTTIN
		false, //SIGTTOU
		false, //SIGIO
		false, //SIGXCPU
		false, //SIGXFSZ
		false, //SIGVTALRM
		false, //SIGPROF
		false, //SIGWINCH
		false, //SIGLOST
		false, //SIGUSR1
		false, //SIGUSR2
		false, //NSIG
};