/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "IPCBuffer.h"
#include <sys/futex.h>

using namespace Duck;
using namespace River;

IPCBuffer::IPCBuffer(Duck::Ptr<Duck::SharedBuffer> buffer):
	m_buffer(std::move(buffer)),
	m_header(m_buffer->ptr<Header>()),
	m_data(m_buffer->ptr<uint8_t>() + sizeof(Header)),
	m_data_size(m_buffer->size() - sizeof(Header))
{
	assert(m_header->magic == IPC_MAGIC);
}

Duck::ResultRet<Duck::Ptr<IPCBuffer>> IPCBuffer::alloc(std::string name, size_t buffer_size) {
	if (buffer_size <= sizeof(Header))
		return Result {"Invalid buffer size"};

	auto buf = TRY(Duck::SharedBuffer::alloc(buffer_size, std::move(name)));
	auto* hdr = buf->ptr<Header>();
	hdr->magic = IPC_MAGIC;
	hdr->read_futex = 0;
	hdr->write_futex = 0;
	hdr->read = 0;
	hdr->write = 0;
	return Duck::Ptr<IPCBuffer>(new IPCBuffer(buf));
}

Duck::ResultRet<Duck::Ptr<IPCBufferReceiver>> IPCBufferReceiver::attach(Duck::Ptr<Duck::SharedBuffer> buffer) {
	if (buffer->size() <= sizeof(Header))
		return Result {"Invalid buffer size"};
	if (buffer->ptr<Header>()->magic != IPC_MAGIC)
		return Result {"Invalid magic"};
	return Duck::Ptr<IPCBufferReceiver>(new IPCBufferReceiver(std::move(buffer)));
}

Result IPCBufferReceiver::recv(const ReadCallback& callback, bool blocking) {
	size_t msg_size;
	auto read = m_header->read.load(); // Since this is in shared memory, we save it to the stack to prevent malicious screwery

	// Acquire read futex
	if(blocking)
		futex_wait(&m_header->read_futex);
	else if(!futex_trywait(&m_header->read_futex))
		return {NO_MESSAGE};

	while(true) {
		// Ensure read head is valid
		if (read > m_data_size) {
			return {INVALID_BUFFER_STATE};
		} else if (read + sizeof(Message) > m_data_size) {
			// If reading a message header would go past the end of the buffer, that means we need to wrap around.
			m_header->read = 0;
			read = 0;
		}

		// Read into output buffer
		msg_size = *((size_t*) (m_data + read));

		// Size of -1 means that we ran out of space when writing the last packet and wrapped around to the beginning
		if (msg_size == -1) {
			m_header->read = 0;
			read = 0;
		} else {
			break;
		}
	}

	// Call callback with message. Check buffer size twice to account for overflows
	if(read > m_data_size || (read + msg_size + sizeof(Message)) > m_data_size)
		return {INVALID_BUFFER_STATE};

	callback(m_data + read + sizeof(Message), msg_size);

	// Move read head
	m_header->read = (read + msg_size + sizeof(Message)) % m_data_size;
	m_header->unread.fetch_sub(msg_size + sizeof(Message), std::memory_order::memory_order_release);
	futex_signal(&m_header->write_futex);

	return Result::SUCCESS;
}

Duck::ResultRet<Duck::Ptr<IPCBufferSender>> IPCBufferSender::attach(Ptr<SharedBuffer> buffer) {
	if (buffer->size() <= sizeof(Header))
		return Result {"Invalid buffer size"};
	if (buffer->ptr<Header>()->magic != IPC_MAGIC)
		return Result {"Invalid magic"};
	return Duck::Ptr<IPCBufferSender>(new IPCBufferSender(std::move(buffer)));
}

Duck::Result IPCBufferSender::send(size_t size, const WriteCallback& callback, bool blocking) {
	if (size > m_data_size - sizeof(Message))
		return {MESSAGE_TOO_LARGE};

	// Set the write futex to 0 just in case we need it
	__atomic_store_n(&m_header->write_futex, 0, __ATOMIC_SEQ_CST);

	auto write = m_header->write.load();

	auto can_write = [&write, size, this] () -> bool {
		const size_t read = m_header->read.load();
		if (read == m_header->write)
			return m_header->unread.load() == 0;
		else if (read > write)
			return write + size + sizeof(Message) < read;
		else
			return (write + size + sizeof(Message) <= m_data_size) || (read >= size + sizeof(Message));
	};

	while (!can_write()) {
		if (!blocking)
			return {NO_BUFFER_SPACE};
		futex_wait(&m_header->write_futex);
	}

	if (write + sizeof(Message) > m_data_size) {
		// If writing a message header would overflow, just wrap around to 0. Reader will know to follow.
		write = 0;
	} else if (write + size + sizeof(Message) > m_data_size) {
		// If writing the header is possible but writing the whole message is not, we need to write a -1 to signify this
		((Message*) (m_data + write))->size = -1;
		write = 0;
	}

	// Write the message header and message
	auto* message = (Message*) (m_data + write);
	message->size = size;
	callback(message->data);

	// Update write head
	write = write + size + sizeof(Message);
	assert(write <= m_data_size);
	if (write == m_data_size)
		write = 0;
	m_header->write = write;

	// Signal reader futex
	m_header->unread.fetch_add(size + sizeof(Message), std::memory_order::memory_order_release);
	futex_signal(&m_header->read_futex);

	return Result::SUCCESS;
}

Duck::Result IPCBufferSender::send(size_t size, const void* val, bool blocking) {
	return send(size, [val, size] (uint8_t* buf) {
		memcpy(buf, val, size);
	}, blocking);
}
