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

#pragma once

#include "types.h"
#include "utility.h"
#include "../memory/kliballoc.h"

namespace kstd {
	template<typename T>
	class circular_queue {
	public:
		circular_queue(size_t capacity): _storage((T*) kcalloc(capacity, sizeof(T))), _capacity(capacity), _size(0) {

		}

		circular_queue(circular_queue<T>& other): _capacity(other._capacity), _size(other._size), _front(other._front), _back(other._back) {
			_storage = (T*) kcalloc(_size, sizeof(T));
			for(size_t i = 0; i < _size; i++)
				new (&_storage[i]) T(other._storage[i]);
		}

		circular_queue(circular_queue<T>&& other) noexcept :
			_storage(other._storage), _capacity(other._capacity), _size(other._size), _front(other._front),
			_back(other._back)
		{
			other._storage = nullptr;
			other._capacity = 0;
			other._size = 0;
			other._front = 0;
			other._back = 0;
		}

		~circular_queue() {
			for(size_t i = 0; i < _size; i++)
				_storage[(_front + i) % _capacity].~T();
			kfree(_storage);
		}

		bool push_back(const T& elem) {
			if(size() == _capacity)
				return false;
			if(_size == 0) {
				_front = 0;
				_back = 0;
			} else {
				_back = (_back + 1) % _capacity;
			}
			new(&_storage[_back]) T(elem);
			_size++;
			return true;
		}

		T pop_front() {
			T ret = _storage[_front];
			_storage[_front].~T();
			_size--;
			if(_size == 0) {
				_front = 0;
				_back = 0;
			} else {
				_front = (_front + 1) % _capacity;
			}
			return ret;
		}

		T pop_back() {
			T ret = _storage[_back];
			_storage[_back].~T();
			_size--;
			if(_size == 0) {
				_front = 0;
				_back = 0;
			} else if(_back == 0) {
				_back = _capacity - 1;
			} else {
				_back--;
			}
			return ret;
		}

		T& front() const {
			return _storage[_front];
		}

		T& back() const {
			return _storage[_back];
		}

		bool empty() const {
			return _size == 0;
		}

		size_t size() const {
			return _size;
		}

		size_t capacity() const {
			return _capacity;
		}

		T* storage() const {
			return _storage;
		}

		circular_queue<T>& operator=(const circular_queue<T>& other) noexcept {
			if(this != &other) {
				for(size_t i = 0; i < _size; i++) _storage[i].~T();
				kfree(_storage);
				_size = other._size;
				_capacity = other._capacity;
				_front = other._front;
				_back = other._back;
				_storage = (T*) kcalloc(_capacity, sizeof(T));
				for (size_t i = 0; i < _size; i++)
					new (&_storage[i]) T(kstd::move(other._storage[i]));
			}
			return *this;
		}

		circular_queue<T>& operator=(circular_queue<T>&& other) noexcept {
			if(this != &other) {
				for(size_t i = 0; i < _size; i++) _storage[i].~T();
				kfree(_storage);
				_size = other._size;
				_capacity = other._capacity;
				_front = other._front;
				_back = other._back;
				_storage = other._storage;

				other._storage = nullptr;
				other._capacity = 0;
				other._size = 0;
				other._front = 0;
				other._back = 0;
			}
			return *this;
		}

	private:
		T* _storage;
		size_t _capacity;
		size_t _size = 0;
		size_t _front = 0;
		size_t _back = 0;
	};
}
