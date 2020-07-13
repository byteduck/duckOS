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

#ifndef DUCKOS_VECTOR_HPP
#define DUCKOS_VECTOR_HPP

#include <kernel/memory/kliballoc.h>
#include "cstddef.h"
#include "cstring.h"
#include "utility.h"

namespace DC {
	template<typename T>
	class vector {
	public:
		vector(): _storage(nullptr), _capacity(0), _size(0) {
			inited = 1;
		}

		vector(size_t capacity, const T& value = T()): _storage((T*) kcalloc(capacity, sizeof(T))), _capacity(capacity) {
			for(size_t i = 0; i < capacity; i++) new (&_storage[i]) T(value);
			inited = 1;
		}

		~vector(){
			if(_storage != nullptr){
				for(size_t i = 0; i < _size; i++) {
					_storage[i].~T();
				}
				kfree(_storage);
			}
		}

		void push_back(const T& elem) {
			if(_size + 1 > _capacity) {
				resize(_capacity == 0 ? 1 : _capacity * 2);
			}
			new (&_storage[_size++]) T(elem);
		}

		void assign(size_t index, const T&& value) {
			new (&_storage[index]) T(DC::move(value));
		}

		void assign(size_t index, const T& value) {
			assign(index, T(value));
		}

		void pop_back() {
			_storage[--_size].~T();
		}

		void resize(size_t new_size) {
			if(new_size == _capacity) return;
			if(new_size < _capacity) {
				for(auto i = _capacity - 1; i >= new_size; i--) {
					_storage[i].~T();
				}
			}
			_capacity = new_size;
			if(_storage == nullptr) _storage = (T*) kcalloc(new_size, sizeof(T));
			else {
				T* tmp_storage = (T*) kcalloc(new_size, sizeof(T));
				for(size_t i = 0; i < _size; i++) {
					new (tmp_storage + i) T((T &&) _storage[i]);
					_storage[i].~T();
				}
				kfree(_storage);
				_storage = tmp_storage;
			}
		}

		void erase(size_t elem) {
			if(elem >= _size) return;
			_storage[elem].~T();
			if(elem != _size - 1) {
				size_t buf_size = (_size - elem - 1) * sizeof(T);
				auto* tmp = new uint8_t[buf_size];
				uint8_t* src_storage = (uint8_t*)((size_t)_storage + sizeof(T) * (elem + 1));
				uint8_t* dest_storage = (uint8_t*)((size_t)_storage + sizeof(T) * elem);
				memcpy(tmp, src_storage, buf_size);
				memcpy(dest_storage, tmp, buf_size);
				delete[] tmp;
			}
			_size--;
		}

		void reserve(size_t new_size) { if(new_size > _capacity) resize(new_size); }
		void shrink_to_fit() { resize(size()); }
		size_t size() const { return _size; }
		size_t capacity() const { return _capacity; }
		bool empty() const { return size() == 0; }

		T& at(size_t index) { return _storage[index]; }
		T& operator[](size_t index) { return _storage[index]; }
		T& front() { return _storage[0]; }
		T& back() { return _storage[_size - 1]; }
		int inited = 0;
	private:
		T* _storage = nullptr;
		size_t _capacity = 0;
		size_t _size = 0;
	};
}


#endif //DUCKOS_VECTOR_HPP
