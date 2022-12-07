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

	Copyright (c) Byteduck 2016-2021. All rights reserved.
*/

#include "SocketFSInode.h"
#include "kernel/KernelMapper.h"
#include <kernel/filesystem/FileDescriptor.h>
#include <kernel/tasking/Thread.h>
#include <kernel/tasking/TaskManager.h>
#include <kernel/kstd/cstring.h>
#include <kernel/filesystem/LinkedInode.h>
#include <kernel/kstd/KLog.h>

SocketFSInode::SocketFSInode(SocketFS& fs, ino_t id, const kstd::string& name, mode_t mode, uid_t uid, gid_t gid):
Inode(fs, id), fs(fs), id(id), name(name), host(0, TaskManager::current_process()->pid())
{
	if(id == 1) { //We're the root inode
		_metadata.mode = MODE_DIRECTORY;
		dir_entry.type = TYPE_DIR;
	} else {
		_metadata.mode = MODE_SOCKET;
		dir_entry.type = TYPE_SOCKET;
	}

	_metadata.uid = uid;
	_metadata.gid = gid;
	_metadata.inode_id = id;

	dir_entry.id = id;
	size_t len = name.length() > NAME_MAXLEN ? NAME_MAXLEN : name.length();
	dir_entry.name_length = len;
	memcpy(dir_entry.name, name.c_str(), len + 1);
}

SocketFSInode::~SocketFSInode() {
}

InodeMetadata SocketFSInode::metadata() {
	if(id != 1)
		return _metadata;

	LOCK(fs.lock);
	InodeMetadata ret = _metadata;
	ret.size = SOCKETFS_PDIR_ENTRY_SIZE + SOCKETFS_CDIR_ENTRY_SIZE;
	for(size_t i = 0; i < fs.sockets.size(); i++)
		ret.size += fs.sockets[i]->dir_entry.entry_length();

	return ret;
}

ino_t SocketFSInode::find_id(const kstd::string& find_name) {
	for(size_t i = 0; i < fs.sockets.size(); i++) {
		if(fs.sockets[i]->name == find_name)
			return fs.sockets[i]->id;
	}
	return -ENOENT;
}

ssize_t SocketFSInode::read(size_t start, size_t length, SafePointer<uint8_t> buffer, FileDescriptor* fd) {
	if(!is_open)
		return -ENOENT;
	if(!fd)
		return -EINVAL;

	LOCK(lock);

	//Find the client that is reading
	SocketFSClient* reader = nullptr;
	auto client_hash = SocketFS::client_hash(fd);

	if(host == client_hash) {
		reader = &host;
	} else {
		for(size_t i = 0; i < clients.size(); i++) {
			if(clients[i] == client_hash) {
				reader = &clients[i];
				break;
			}
		}
	}

	if(!reader) {
		//Couldn't find the client reading...
		return -EIO;
	}

	if(length > reader->data_queue->size())
		length = reader->data_queue->size();

	//Read into the buffer from the queue
	auto& queue = reader->data_queue;
	for(size_t i = 0; i < length; i++) {
		buffer.set(i, queue->front());
		queue->pop_front();
	}

	reader->_blocker.set_ready(true);

	return length;
}

ResultRet<kstd::Arc<LinkedInode>> SocketFSInode::resolve_link(const kstd::Arc<LinkedInode>& base, const User& user, kstd::Arc<LinkedInode>* parent_storage, int options, int recursion_level) {
	return Result(-ENOLINK);
}

ssize_t SocketFSInode::read_dir_entry(size_t start, SafePointer<DirectoryEntry> buffer, FileDescriptor* fd) {
	if(id != 1)
		return -ENOTDIR;

	if(start == 0) {
		DirectoryEntry ent(id, TYPE_DIR, ".");
		buffer.set(ent);
		return SOCKETFS_CDIR_ENTRY_SIZE;
	} else if(start == SOCKETFS_CDIR_ENTRY_SIZE) {
		DirectoryEntry ent(0, TYPE_DIR, "..");
		buffer.set(ent);
		return SOCKETFS_PDIR_ENTRY_SIZE;
	}

	size_t cur_index = SOCKETFS_CDIR_ENTRY_SIZE + SOCKETFS_PDIR_ENTRY_SIZE;
	LOCK(fs.lock);

	for(size_t i = 0; i < fs.sockets.size(); i++) {
		auto& e = fs.sockets[i];
		if(e->is_open) {
			if(cur_index >= start) {
				buffer.set(e->dir_entry);
				return e->dir_entry.entry_length();
			}
			cur_index += e->dir_entry.entry_length();
		}
	}

	return 0;
}

ssize_t SocketFSInode::write(size_t start, size_t length, SafePointer<uint8_t> buf, FileDescriptor* fd) {
	if(!is_open)
		return -EIO;
	if(!fd)
		return -EINVAL;

	lock.acquire();
	auto packet = SafePointer<SocketFSPacket>(buf).get();
	auto packet_data = SafePointer<uint8_t>(buf.raw() + sizeof(SocketFSPacket), buf.is_user());

	SocketFSClient* recipient = nullptr;
	SocketFSClient* sender = nullptr;
	bool is_broadcast = packet.type == SOCKETFS_TYPE_BROADCAST;

	//Find the client that the packet is coming from
	auto client_hash = SocketFS::client_hash(fd);
	if(host == client_hash) {
		sender = &host;
	} else {
		for(size_t i = 0; i < clients.size(); i++) {
			if(clients[i] == client_hash) {
				sender = &clients[i];
				break;
			}
		}
	}

	if(!sender) {
		//Couldn't find the client it came from...
		lock.release();
		return -EIO;
	}

	if(is_broadcast && sender == &host) {
		//If it's a broadcast, send it to all clients
		for(size_t i = 0; i < clients.size(); i++) {
			//Share shm with client if we need to
			if(packet.shm_id)
				TaskManager::current_process()->sys_shmallow(packet.shm_id, clients[i].pid, packet.shm_perms);
			//We don't care about errors here, we should just continue sending it to the rest of the clients
			write_packet(clients[i], SOCKETFS_TYPE_MSG, sender->id, packet.length, packet.shm_id, packet.shm_perms, packet_data, fd->nonblock());
		}
		lock.release();
		return SUCCESS;
	} else if(sender == &host) {
		//Find the client this packet has to go to
		for(size_t i = 0; i < clients.size(); i++) {
			if(clients[i] == packet.recipient) {
				recipient = &clients[i];
				break;
			}
		}
	} else if(sender != &host) {
		//Clients can only send packets to the host
		if(packet.recipient != SOCKETFS_RECIPIENT_HOST) {
			lock.release();
			return -EINVAL;
		}
		recipient = &host;
	}

	if(!recipient) {
		lock.release();
		return -EINVAL; //No such recipient
	}

	//Share shm with recipient if we need to
	if(packet.shm_id) {
		int shm_res = TaskManager::current_process()->sys_shmallow(packet.shm_id, recipient->pid, packet.shm_perms);
		if (shm_res != SUCCESS) {
			lock.release();
			return shm_res;
		}
	}

	//Finally, write the packet to the correct queue
	auto res = write_packet(*recipient, SOCKETFS_TYPE_MSG, sender->id, packet.length, packet.shm_id, packet.shm_perms, packet_data, fd->nonblock());
	lock.release();
	return res.code();
}

Result SocketFSInode::add_entry(const kstd::string& add_name, Inode& inode) {
	return Result(-EINVAL);
}

ResultRet<kstd::Arc<Inode>> SocketFSInode::create_entry(const kstd::string& create_name, mode_t mode, uid_t uid, gid_t gid) {
	if(id != 1)
		return Result(-ENOTDIR);
	LOCK(fs.lock);

	mode = (mode & 0x0FFFu) | MODE_SOCKET;

	//Hash the name for use in the unique identifier
	uint16_t hash = 7;
	for (int i = 0; i < create_name.length(); i++) {
		hash = hash * 31 + create_name[i];
	}

	Process* proc = TaskManager::current_process();
	ino_t create_id = SocketFS::get_inode_id(proc->pid(), hash);

	//Make sure that nothing exists with the same name / id
	for(size_t i = 0; i < fs.sockets.size(); i++) {
		if(fs.sockets[i]->name == create_name || fs.sockets[i]->id == create_id)
			return Result(-EEXIST);
	}

	//Create the socket and return it
	auto new_inode = kstd::make_shared<SocketFSInode>(fs, create_id, create_name, mode, uid, gid);
	fs.sockets.push_back(new_inode);
	return static_cast<kstd::Arc<Inode>>(new_inode);
}

Result SocketFSInode::remove_entry(const kstd::string& remove_name) {
	return Result(-EACCES);
}

Result SocketFSInode::truncate(off_t length) {
	return Result(-EINVAL);
}

Result SocketFSInode::chmod(mode_t new_mode) {
	LOCK(lock);
	_metadata.mode = new_mode;
	return Result(SUCCESS);
}

Result SocketFSInode::chown(uid_t new_uid, gid_t new_gid) {
	LOCK(lock);
	_metadata.uid = new_uid;
	_metadata.gid = new_gid;
	return Result(SUCCESS);
}

void SocketFSInode::open(FileDescriptor& fd, int options) {
	LOCK(lock);

	auto client_hash = SocketFS::client_hash(&fd);

	//If nobody has taken ownership of this socket, make this file descriptor the owner
	if(!host && (options & O_CREAT)) {
		host.id = client_hash;
		host.pid = TaskManager::current_process()->pid();
		return;
	}

	//Add the client and send the connect message to the host
	clients.push_back(SocketFSClient(client_hash, fd.owner()));
	write_packet(host, SOCKETFS_TYPE_MSG_CONNECT, client_hash, 0, 0, 0, KernelPointer<uint8_t>(nullptr), true);
}

void SocketFSInode::close(FileDescriptor& fd) {
	LOCK(lock);

	auto client_hash = SocketFS::client_hash(&fd);

	if(host == client_hash) {
		//Remove the socket
		is_open = false;
		ScopedLocker __locker2(fs.lock);
		for(size_t i = 0; i < fs.sockets.size(); i++) {
			if(fs.sockets[i].get() == this) {
				fs.sockets.erase(i);
				return;
			}
		}
		KLog::warn("SocketFS", "Socket %d was closed by host but couldn't find an entry to remove!", id);
		return;
	}

	for(size_t i = 0; i < clients.size(); i++) {
		if(clients[i] == client_hash) {
			write_packet(host, SOCKETFS_TYPE_MSG_DISCONNECT, client_hash, 0, 0, 0, KernelPointer<uint8_t>(nullptr), true);
			clients.erase(i);
			break;
		}
	}
}

bool SocketFSInode::can_read(const FileDescriptor& fd) {
	auto client_hash = SocketFS::client_hash(&fd);

	if(host == client_hash)
		return !host.data_queue->empty();

	for(size_t i = 0; i < clients.size(); i++) {
		if(clients[i] == client_hash)
			return !clients[i].data_queue->empty();
	}

	return false;
}

Result SocketFSInode::write_packet(SocketFSClient& client, int type, sockid_t sender, size_t length, int shm_id, int shm_perms, SafePointer<uint8_t> buffer, bool nonblock) {
	//If there's room in the buffer, block (if O_NONBLOCK isn't set)
	while(sizeof(SocketFSPacket) + length + client.data_queue->size() > SOCKETFS_MAX_BUFFER_SIZE) {
		if(!nonblock) {
			lock.release();
			client._blocker.set_ready(false);
			TaskManager::current_thread()->block(client._blocker);
			lock.acquire();
		} else {
			return Result(-ENOSPC);
		}
	}

	//Acquire the lock
	LOCK(client._lock);

	//Write the packet header
	SocketFSPacket packet_header = {type, sender, TaskManager::current_process()->pid(), length, shm_id, shm_perms};
	auto* data = (const uint8_t*) &packet_header;
	for(size_t i = 0; i < sizeof(SocketFSPacket); i++)
		client.data_queue->push_back(*data++);

	//Write the packet body
	for(size_t i = 0; i < length; i++)
		client.data_queue->push_back(buffer.get(i));

	return Result(SUCCESS);
}
