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

#ifndef DUCKOS_QUEUE_HPP
#define DUCKOS_QUEUE_HPP

#include <kernel/memory/kliballoc.h>
#include "cstddef.h"
#include "cstring.h"
#include "utility.h"

namespace DC {
	template<typename T>
	class queue {
	public:
		queue(): _storage(nullptr), _capacity(0), _size(0) {

		}

		queue(queue<T>& other): _capacity(other._capacity), _size(other._size), _front(other.front), _back(other._back) {
			_storage = (T*) kcalloc(_size, sizeof(T));
			for(size_t i = 0; i < _size; i++)
				new (&_storage[i]) T(other._storage[i]);
		}

		queue(queue<T>&& other): _capacity(other._capacity), _size(other._size), _front(other.front), _back(other._back) {
			_storage = (T*) kcalloc(_size, sizeof(T));
			for(size_t i = 0; i < _size; i++)
				new (&_storage[i]) T(DC::move(other._storage[i]));
		}

		~queue() {
			if(_front > _back) {
				for(size_t i = _back; i <= _front; i++)
					_storage[i].~T();
			} else if(_back > _front) {
				for(size_t i = _front; i <= _back; i++)
					_storage[i].~T();
			}

			kfree(_storage);
		}

		void push(const T& elem) {
			if(size() == _capacity)
				realloc(_capacity ? _capacity * 2 : 1);

			if(_size == 0) {
				_front = 0;
				_back = 0;
			} else {
				_back = (_back + 1) % _capacity;
			}

			new(&_storage[_back]) T(elem);

			_size++;
		}

		void realloc(size_t new_capacity) {
			if(new_capacity == _capacity) return;

			if(_storage == nullptr) {
				_capacity = new_capacity;
				_storage = (T*) kcalloc(_capacity, sizeof(T));
				return;
			}

			if(new_capacity < _size) {
				for(size_t i = new_capacity; i < _size; i++)
					_storage[i].~T();
				_size = new_capacity;
			}

			T* tmp_storage = (T*) kcalloc(new_capacity, sizeof(T));
			for(size_t i = 0; i < _size; i++) {
				new (&tmp_storage[i]) T(_storage[i]);
				_storage[i].~T();
			}

			kfree(_storage);
			_storage = tmp_storage;
			_capacity = new_capacity;
		}

		void pop() {
			_storage[_front].~T();
			_size--;

			if(_size == 0) {
				_front = 0;
				_back = 0;
			} else {
				_front = (_front + 1) % _capacity;
			}
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

		queue<T>& operator=(const queue<T>& other) noexcept {
			if(this != &other) {
				for(size_t i = 0; i < _size; i++) _storage[i].~T();
				kfree(_storage);
				_size = other._size;
				_capacity = other._capacity;
				_front = other._front;
				_back = other._back;
				_storage = (T*) kcalloc(_capacity, sizeof(T));
				for (size_t i = 0; i < _size; i++)
					new (&_storage[i]) T(DC::move(other._storage[i]));
			}
			return *this;
		}

		queue<T>& operator=(queue<T>&& other) noexcept {
			if(this != &other) {
				for(size_t i = 0; i < _size; i++) _storage[i].~T();
				kfree(_storage);
				_size = other._size;
				_capacity = other._capacity;
				_front = other._front;
				_back = other._back;
				_storage = (T*) kcalloc(_capacity, sizeof(T));
				for (size_t i = 0; i < _size; i++)
					new (&_storage[i]) T(DC::move(other._storage[i]));
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

#endif //DUCKOS_QUEUE_HPP
