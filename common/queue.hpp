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
		queue(size_t capacity = 0): _storage(new T[capacity]), _capacity(capacity), _size(0) {

		}

		~queue() {
			for(size_t i = 0; i < _capacity; i++)
				_storage[i].~T();
			delete _storage;
		}

		void push(const T& elem) {
			if(size() == _capacity) {
				if(_capacity == 0) _capacity = 1;
				else _capacity *= 2;
				T* tmp_storage = (T*) kcalloc(_capacity, sizeof(T));
				for(size_t i = 0; i < _size; i++) {
					new (tmp_storage + i) T((T &&) _storage[i]);
					_storage[i].~T();
				}
				kfree(_storage);
				_storage = tmp_storage;
			}
			if((int) _front == -1) _front = 0;
			_back++;
			_back = _back % _capacity;
			_storage[_back] = DC::move(elem);
			_size++;
		}

		T& pop() {
			T& ret = _storage[_front];
			if(_front == _back) {
				_front = -1;
				_back = -1;
			} else {
				_front++;
				_front = _front % _capacity;
			}
			_size--;
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
	private:
		T* _storage;
		size_t _capacity;
		size_t _size = 0;
		size_t _front = -1;
		size_t _back = -1;
	};
}

#endif //DUCKOS_QUEUE_HPP
