/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include <libduck/SharedBuffer.h>
#include <functional>
#include <atomic>

namespace River {
	class IPCBuffer {
	public:
		struct Message {
			size_t size;
			uint8_t data[];
		};

		enum ResultCode {
			NO_MESSAGE,
			INVALID_BUFFER_STATE,
			NO_BUFFER_SPACE,
			MESSAGE_TOO_LARGE
		};

		static constexpr size_t default_buffer_size = PAGE_SIZE * 8;

		static Duck::ResultRet<Duck::Ptr<IPCBuffer>> alloc(std::string name, size_t buffer_size = default_buffer_size);

		Duck::Ptr<Duck::SharedBuffer> buffer() const { return m_buffer; }

	protected:
		IPCBuffer(Duck::Ptr<Duck::SharedBuffer> buffer);

		static constexpr int IPC_MAGIC = 0x42069;
		struct Header {
			int magic;
			int read_futex;
			int write_futex;
			std::atomic<size_t> unread, read, write;
		};

		Duck::Ptr<Duck::SharedBuffer> m_buffer;
		uint8_t* m_data;
		size_t m_data_size;
		Header* m_header;
	};

	class IPCBufferReceiver: public IPCBuffer {
	public:
		static Duck::ResultRet<Duck::Ptr<IPCBufferReceiver>> attach(Duck::Ptr<Duck::SharedBuffer> buffer);

		using ReadCallback = std::function<void(const uint8_t*, size_t)>;
		Duck::Result recv(const ReadCallback& callback, bool blocking = true);

	private:
		IPCBufferReceiver(Duck::Ptr<Duck::SharedBuffer> buffer): IPCBuffer(std::move(buffer)) {};
	};

	class IPCBufferSender: public IPCBuffer {
	public:

		static Duck::ResultRet<Duck::Ptr<IPCBufferSender>> attach(Duck::Ptr<Duck::SharedBuffer> buffer);

		using WriteCallback = std::function<void(uint8_t*)>;
		Duck::Result send(size_t size, const WriteCallback& callback, bool blocking = true);
		Duck::Result send(size_t size, const void* val, bool blocking = true);

	private:

		IPCBufferSender(Duck::Ptr<Duck::SharedBuffer> buffer): IPCBuffer(std::move(buffer)) {};
	};
}
