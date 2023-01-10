/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "../tasking/Process.h"
#include "../memory/SafePointer.h"
#include "../filesystem/FileDescriptor.h"
#include "../tasking/PollBlocker.h"

int Process::sys_poll(UserspacePointer<pollfd> pollfd, nfds_t nfd, int timeout) {
	//Build the list of PollBlocker::PollFDs
	kstd::vector<PollBlocker::PollFD> polls;
	polls.reserve(nfd);
	for(nfds_t i = 0; i < nfd; i++) {
		auto poll = pollfd.get(i);
		//Make sure the fd is valid. If not, set revents to POLLINVAL
		if(poll.fd < 0 || poll.fd >= (int) _file_descriptors.size() || !_file_descriptors[poll.fd]) {
			poll.revents = POLLINVAL;
		} else {
			poll.revents = 0;
			polls.push_back({poll.fd, _file_descriptors[poll.fd], poll.events});
		}
		pollfd.set(i, poll);
	}

	//Block
	PollBlocker blocker(polls, Time(0, timeout * 1000));
	TaskManager::current_thread()->block(blocker);
	if(blocker.was_interrupted())
		return -EINTR;

	//Set appropriate revent
	for(nfds_t i = 0; i < nfd; i++) {
		auto poll = pollfd.get(i);
		if(poll.fd == blocker.polled) {
			poll.revents = blocker.polled_revent;
			pollfd.set(i, poll);
			break;
		}
	}

	return SUCCESS;
}