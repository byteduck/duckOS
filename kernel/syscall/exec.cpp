/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include "../tasking/Process.h"
#include "../memory/SafePointer.h"
#include "../tasking/ProcessArgs.h"
#include "../filesystem/VFS.h"
#include "../terminal/VirtualTTY.h"

int Process::exec(const kstd::string& filename, ProcessArgs* args) {
	//Scoped so that stack variables are cleaned up before yielding
	{
		//Create the new process
		auto R_new_proc = Process::create_user(filename, _user, args, _pid, _ppid);
		if (R_new_proc.is_error())
			return R_new_proc.code();
		auto new_proc = R_new_proc.value();

		//Properly set new process's PID, blocker, and stdout/in/err
		new_proc->_user = _user;
		new_proc->_pgid = _pgid;
		new_proc->_sid = _sid;
		if (_kernel_mode) {
			//Kernel processes have no file descriptors, so we need to initialize them
			auto ttydesc = kstd::make_shared<FileDescriptor>(VirtualTTY::current_tty());
			ttydesc->set_owner(_self_ptr);
			ttydesc->set_options(O_RDWR);
			_file_descriptors.resize(0); //Just in case
			_file_descriptors.push_back(ttydesc);
			_file_descriptors.push_back(ttydesc);
			_file_descriptors.push_back(ttydesc);
			_cwd = args->working_dir;
		}

		//Give new process all of the file descriptors
		new_proc->_file_descriptors.resize(_file_descriptors.size());
		int last_fd = 0;
		for (size_t i = 0; i < _file_descriptors.size(); i++) {
			if (_file_descriptors[i] && !_file_descriptors[i]->cloexec()) {
				last_fd = i;
				new_proc->_file_descriptors[i] = _file_descriptors[i];
				new_proc->_file_descriptors[i]->set_owner(new_proc);
			}
		}
		//Trim off null file descriptors
		new_proc->_file_descriptors.resize(last_fd + 1);

		_file_descriptors.resize(0);

		//Manually delete because we won't return from here and we need to clean up resources
		//TODO a better way of doing this
		delete args;
		filename.~string();

		//Add the new process to the process list
		auto main_thread = _threads[_pid];
		if(main_thread) {
			remove_thread(main_thread);
			main_thread->_tid = -1;
			insert_thread(main_thread);
		}
		_pid = -1;
		_ppid = 0;
		TaskManager::add_process(new_proc);
	}

	die();
	return -1;
}

int Process::sys_execve(UserspacePointer<char> filename, UserspacePointer<char*> argv, UserspacePointer<char*> envp) {
	auto* args = new ProcessArgs(_cwd);
	if(argv) {
		int i = 0;
		while(argv.get(i)) {
			args->argv.push_back(UserspacePointer<char>(argv.get(i)).str());
			i++;
		}
	}
	if(envp) {
		int i = 0;
		while(envp.get(i)) {
			args->env.push_back(UserspacePointer<char>(envp.get(i)).str());
			i++;
		}
	}
	return exec(filename.str(), args);
}