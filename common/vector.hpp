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
		}

		explicit vector(size_t size, const T& value = T()): _capacity(size), _size(size) {
			_storage = (T*) kcalloc(size, sizeof(T));
			for(size_t i = 0; i < size; i++)
				new (&_storage[i]) T(value);
		}

		vector(vector<T>& other): _capacity(other._capacity), _size(other._size) {
			_storage = (T*) kcalloc(_size, sizeof(T));
			for(size_t i = 0; i < _size; i++)
				new (&_storage[i]) T(other._storage[i]);
		}

		vector(vector<T>&& other): _capacity(other._capacity), _size(other._size) {
			_storage = (T*) kcalloc(_size, sizeof(T));
			for(size_t i = 0; i < _size; i++)
				new (&_storage[i]) T(DC::move(other._storage[i]));
		}

		~vector(){
			if(_storage != nullptr) {
				for(size_t i = 0; i < _size; i++) _storage[i].~T();
				kfree(_storage);
			}
		}

		void push_back(const T& elem) {
			if(_size + 1 > _capacity) {
				realloc(_capacity == 0 ? 1 : _capacity * 2);
			}
			new (&_storage[_size++]) T(elem);
		}

		void resize(size_t new_size) {
			if(_size == new_size) return;
			if(new_size > _capacity) realloc(new_size);
			if(new_size > _size) {
				for(size_t i = _size; i < new_size; i++)
					new (&_storage[i - 1]) T();
			} else {
				for(size_t i = new_size; i < _size; i++)
					_storage[i].~T();
			}
			_size = new_size;
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

		void erase(size_t elem) {
			if(elem >= _size) return;
			_storage[elem].~T();
			for(size_t i = elem; i < _size - 1; i++)
				_storage[i] = DC::move(_storage[i + 1]);
			_size--;
		}

		void reserve(size_t new_capacity) {
			if(new_capacity > _capacity)
				realloc(new_capacity);
		}

		void shrink_to_fit() {
			realloc(size());
		}

		size_t size() const {
			return _size;
		}

		size_t capacity() const {
			return _capacity;
		}

		bool empty() const {
			return size() == 0;
		}

		T& at(size_t index) {
			return _storage[index];
		}

		T& operator[](size_t index) {
			return _storage[index];
		}

		vector<T>& operator=(const vector<T>& other) {
			if(this != &other) {
				for(size_t i = 0; i < _size; i++) _storage[i].~T();
				kfree(_storage);
				_size = other._size;
				_capacity = other._capacity;
				_storage = (T*) kcalloc(_capacity, sizeof(T));
				for (size_t i = 0; i < _size; i++)
					new (&_storage[i]) T(other._storage[i]);
			}
			return *this;
		}

		vector<T>& operator=(vector<T>&& other) noexcept {
			if(this != &other) {
				for(size_t i = 0; i < _size; i++) _storage[i].~T();
				kfree(_storage);
				_size = other._size;
				_capacity = other._capacity;
				_storage = (T*) kcalloc(_capacity, sizeof(T));
				for (size_t i = 0; i < _size; i++)
					new (&_storage[i]) T(DC::move(other._storage[i]));
			}
			return *this;
		}

		T& front() {
			return _storage[0];
		}

		T& back() {
			return _storage[_size - 1];
		}

	private:
		T* _storage = nullptr;
		size_t _capacity = 0;
		size_t _size = 0;
	};
}


#endif //DUCKOS_VECTOR_HPP
