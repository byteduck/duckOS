/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "../tasking/Process.h"
#include "../tasking/TaskManager.h"
#include "../memory/SafePointer.h"
#include "../terminal/TTYDevice.h"

int Process::sys_getsid(pid_t pid) {
	if(pid == 0)
		return _sid;
	auto proc = TaskManager::process_for_pid(pid);
	if(proc.is_error())
		return -ESRCH;
	if(proc.value()->_sid != _sid)
		return -EPERM;
	return proc.value()->_sid;
}

int Process::sys_setsid() {
	//Make sure there's no other processes in the group
	auto* procs = TaskManager::process_list();
	for(int i = 0; i < procs->size(); i++) {
		auto c_proc = procs->at(i);
		if(c_proc->_pgid == _pid)
			return -EPERM;
	}

	_sid = _pid;
	_pgid = _pid;
	_tty.reset();
	return _sid;
}

int Process::sys_getpgid(pid_t pid) {
	if(pid == 0)
		return _pgid;
	auto proc = TaskManager::process_for_pid(pid);
	if(proc.is_error())
		return -ESRCH;
	return proc.value()->_pgid;
}

int Process::sys_getpgrp() {
	return _pgid;
}

int Process::sys_setpgid(pid_t pid, pid_t new_pgid) {
	if(pid < 0)
		return -EINVAL;

	//Validate specified pid
	auto proc = pid ? TaskManager::process_for_pid(pid) : ResultRet<Process*>(_self_ptr);
	if(proc.is_error() || (proc.value()->_pid != _pid && proc.value()->_ppid != _pid))
		return -ESRCH; //Process doesn't exist or is not self or child
	if(proc.value()->_ppid == _pid && proc.value()->_sid != _sid)
		return -EPERM; //Process is a child but not in the same session
	if(proc.value()->_pid == proc.value()->_sid)
		return -EPERM; //Process is session leader

	//If new_pgid is 0, use the pid of the target process
	if(!new_pgid)
		new_pgid = proc.value()->_pid;

	//Make sure we're not switching to another session
	pid_t new_sid = _sid;
	auto* procs = TaskManager::process_list();
	for(int i = 0; i < procs->size(); i++) {
		auto c_proc = procs->at(i);
		if(c_proc->_pgid == new_pgid)
			new_sid = c_proc->_sid;
	}
	if(new_sid != _sid)
		return -EPERM;

	proc.value()->_pgid = new_pgid;
	return SUCCESS;
}

int Process::sys_setuid(uid_t uid) {
	if(!_user.can_setuid() && uid != _user.uid && uid != _user.euid)
		return -EPERM;

	_user.uid = uid;
	_user.euid = uid;
	return SUCCESS;
}

int Process::sys_seteuid(uid_t euid) {
	if(!_user.can_setuid() && euid != _user.uid && euid != _user.euid)
		return -EPERM;

	_user.euid = euid;
	return SUCCESS;
}

uid_t Process::sys_getuid() {
	return _user.uid;
}

uid_t Process::sys_geteuid() {
	return _user.euid;
}

int Process::sys_setgid(gid_t gid) {
	if(!_user.can_setgid() && gid != _user.gid && gid != _user.egid)
		return -EPERM;

	_user.gid = gid;
	_user.egid = gid;
	return SUCCESS;
}

int Process::sys_setegid(gid_t egid) {
	if(!_user.can_setgid() && egid != _user.gid && egid != _user.egid)
		return -EPERM;

	_user.egid = egid;
	return SUCCESS;
}

gid_t Process::sys_getgid() {
	return _user.gid;
}

gid_t Process::sys_getegid() {
	return _user.egid;
}

int Process::sys_setgroups(size_t count, UserspacePointer<gid_t> gids) {
	if(count < 0)
		return -EINVAL;
	if(!_user.can_setgid())
		return -EPERM;

	if(!count) {
		_user.groups.resize(0);
		return SUCCESS;
	}

	_user.groups.resize(count);
	for(size_t i = 0; i < count; i++) _user.groups[i] = gids.get(i);
	return SUCCESS;
}

int Process::sys_getgroups(int count, UserspacePointer<gid_t> gids) {
	if(count < 0)
		return -EINVAL;
	if(count == 0)
		return _user.groups.size();
	if(count <  _user.groups.size())
		return -EINVAL;
	for(size_t i = 0; i <  _user.groups.size(); i++) gids.set(i, _user.groups[i]);
	return SUCCESS;
}

mode_t Process::sys_umask(mode_t new_mask) {
	auto ret = _umask;
	_umask = new_mask & 0777;
	return ret;
}