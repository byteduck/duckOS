/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "../tasking/Process.h"
#include "../memory/SafePointer.h"
#include "../filesystem/LinkedInode.h"

int Process::sys_getcwd(UserspacePointer<char> buf, size_t length) {
	if(_cwd->name().length() > length)
		return -ENAMETOOLONG;
	kstd::string path = _cwd->get_full_path();
	buf.write(path.c_str(), min(length, path.length()));
	buf.set(path.length(), '\0');
	return 0;
}