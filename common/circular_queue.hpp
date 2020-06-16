#ifndef DUCKOS_CIRCULAR_QUEUE_HPP
#define DUCKOS_CIRCULAR_QUEUE_HPP

#include "cstddef.h"
#include "utility.h"
#include "stdlib.h"

namespace DC {
	template<typename T>
	class circular_queue {
	public:
		circular_queue(size_t size): _storage(new T[size]), _capacity(size) {
		}

		~circular_queue() {
			for(auto i = 0; i < _capacity; i++)
				_storage[i].~T();
			delete _storage;
		}

		bool push(const T& elem) {
			if(size() == _capacity) return false;
			if(_front == -1) _front = 0;
			_back = (++_back) % _capacity;
			_storage[_back] = DC::move(elem);
			_size++;
			return true;
		}

		T& pop_front() {
			T& ret = _storage[_front];
			if(_front == _back) {
				_front = -1;
				_back = -1;
			} else _front = (++_front) % _capacity;
			_size--;
			return ret;
		}

		T& pop_back() {
			T& ret = _storage[_back];
			if(_front == _back) {
				_front = -1;
				_back = -1;
			} if(_back == 0) {
				_back = _capacity - 1;
			} else _back--;
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
		size_t _size;
		size_t _front = -1;
		size_t _back = -1;
	};
}

#endif //DUCKOS_CIRCULAR_QUEUE_HPP
