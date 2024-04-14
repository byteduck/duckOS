/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libduck/Args.h>

const char* signal_names[] = {
	"0",
	"HUP",
	"INT",
	"QUIT",
	"ILL",
	"TRAP",
	"ABRT",
	"EMT",
	"FPE",
	"KILL",
	"BUS",
	"SEGV",
	"SYS",
	"PIPE",
	"ALRM",
	"TERM",
	"URG",
	"STOP",
	"TSTP",
	"CONT",
	"CHLD",
	"TTIN",
	"TTOU",
	"IO",
	"XCPU",
	"XFSZ",
	"VTALRM",
	"PROF",
	"WINCH",
	"LOST",
	"USR1",
	"USR2",
	"NSIG"
};

int name_to_signal(const char* name) {
	for(int i = 0; i < NSIG; i++) {
		if(!strcmp(signal_names[i], name))
			return i;
	}
	return 0;
}

pid_t pid;
std::string sig = "TERM";

int main(int argc, char** argv) {
	Duck::Args args;
	args.add_positional(pid, true, "PID", "The process ID.");
	args.add_named(sig, "s", "signal", "The signal to send, either a name or ID.");
	args.parse(argc, argv);

	int sigid = -strtol(sig.c_str(), nullptr, 0);
	if (sigid == 0)
		sigid = name_to_signal(sig.c_str());
	if (sigid <= 0 || sigid >= NSIG) {
		fprintf(stderr, "kill: Invalid signal\n");
		return EXIT_FAILURE;
	}

	if(kill(pid, sigid) == -1) {
		perror("kill");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}