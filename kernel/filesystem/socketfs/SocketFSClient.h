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

#ifndef DUCKOS_SOCKETFSCLIENT_H
#define DUCKOS_SOCKETFSCLIENT_H

#include <kernel/tasking/Process.h>

class SocketFSClient {
public:
	SocketFSClient(kstd::shared_ptr<Process> process, pid_t pid);

	kstd::shared_ptr<Process> process;
	pid_t pid;
	kstd::shared_ptr<kstd::queue<uint8_t>> data_queue;
	BooleanBlocker _blocker;
	SpinLock _lock;
};

#endif //DUCKOS_SOCKETFSCLIENT_H
