/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include <atomic>
#include <cassert>
#include "SharedBuffer.h"

namespace Duck {
	/**
	 * This class is meant to be used in multithreaded or IPC applications where a circular queue is needed.
	 * The queue can be pushed to and popped from atomically without worry of synchronization.
	 * One thread can push to the queue, and one thread can pop.
	 */
	template<typename T, int Size>
	class AtomicCircularQueue {
	public:
		AtomicCircularQueue(): m_buffer(nullptr), m_queue(nullptr) {}

		static AtomicCircularQueue attach(Ptr<SharedBuffer> buffer) {
			return AtomicCircularQueue(buffer);
		}

		static ResultRet<AtomicCircularQueue> alloc(std::string name) {
			auto buffer = TRY(SharedBuffer::alloc(sizeof(AtomicCircularQueueStruct), std::move(name)));
			new (buffer->ptr<AtomicCircularQueueStruct>()) AtomicCircularQueueStruct;
			return AtomicCircularQueue(buffer);
		}

		bool full() {
			return ((m_queue->front.load() - 1) % Size) == (m_queue->back.load() % Size);
		}

		bool empty() {
			return m_queue->front.load() == m_queue->back.load();
		}

		/** Tries to push a value to the queue. Returns true if successful, or false if no space was available. **/
		bool push(const T& value) {
			if(full())
				return false;
			auto back = m_queue->back.load() % Size;
			new (&m_queue->storage[back]) T(value);
			m_queue->back.fetch_add(1);
			return true;
		}

		/** Pushes a value to the queue, waiting until space is available. **/
		void push_wait(const T& value) {
			while(!push(value))
				usleep(1);
		}

		/** Pops a value from the queue, if available. **/
		std::optional<T> pop() {
			if(empty())
				return std::nullopt;
			auto front = m_queue->front.load() % Size;
			auto ret = std::move(m_queue->storage[front]);
			m_queue->front.fetch_add(1);
			return ret;
		}

		/** Pops a value from the queue, waiting until one is available. **/
		T pop_wait() {
			while(true) {
				auto res = pop();
				if(res.has_value())
					return res.value();
			}
		}

		Ptr<SharedBuffer> buffer() {
			return m_buffer;
		}

	private:
		AtomicCircularQueue(Ptr<SharedBuffer> buffer):
				m_buffer(buffer),
				m_queue(buffer->ptr<AtomicCircularQueueStruct>())
		{
			assert(buffer->size() >= sizeof(AtomicCircularQueueStruct));
		}

		struct AtomicCircularQueueStruct {
		public:

			/*
			 * Instead of being wrapped around automatically like a regular queue, front and back can only be increased.
			 * This means that in order to get the "real" position of the front and back, we have to mod them by Size.
			 *
			 * This way, we know that the queue is empty if front == back and full if (front - 1) % Size == back,
			 * instead of having to keep track of size separately, which would complicate things.
			 */

			std::atomic<size_t> front = 0; /* Points to the next element to be popped off the queue. */
			std::atomic<size_t> back = 0; /* Points to where the next element will be pushed onto the queue. */

			T storage[Size];
		};

		Ptr<SharedBuffer> m_buffer;
		AtomicCircularQueueStruct* m_queue;
	};
}
