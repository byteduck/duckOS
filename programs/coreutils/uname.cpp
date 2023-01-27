/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include <libduck/Args.h>
#include <sys/utsname.h>

bool all = false;
bool sysname = false;
bool nodename = false;
bool release = false;
bool version = false;
bool machine = false;

int main(int argc, char** argv) {
	Duck::Args args;
	args.add_flag(all, "a", std::nullopt, "Display all information.");
	args.add_flag(machine, "m", std::nullopt, "Display the machine hardware name.");
	args.add_flag(nodename, "n", std::nullopt, "Display the name of the machine.");
	args.add_flag(sysname, "s", std::nullopt, "Display the kernel name.");
	args.add_flag(version, "v", std::nullopt, "Display the kernel version.");
	args.add_flag(release, "r", std::nullopt, "Display the kernel release.");
	args.add_flag(sysname, "o", std::nullopt, "An alias for the -s flag.");
	args.parse(argc, argv);

	if(argc == 1)
		sysname = true;

	utsname buf;
	if(uname(&buf)) {
		perror("uname");
		return EXIT_FAILURE;
	}

	std::vector<char*> to_print;

	if(all || sysname)
		to_print.push_back(buf.sysname);
	if(all || nodename)
		to_print.push_back(buf.nodename);
	if(all || release)
		to_print.push_back(buf.release);
	if(all || version)
		to_print.push_back(buf.version);
	if(all || machine)
		to_print.push_back(buf.machine);

	for(int i = 0; i < to_print.size(); i++) {
		if(i != 0)
			putchar(' ');
		printf("%s", to_print[i]);
	}
	printf("\n");

	return EXIT_SUCCESS;
}