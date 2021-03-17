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

#include "string.h"

namespace kstd {
	string::string(): _size(1), _length(0), _cstring(new char[1]) {
		_cstring[0] = '\0';
	}

	string::string(const string& string) {
		_length = string._length;
		_size = string._size;
		_cstring = new char[_size];
		strcpy(_cstring, string._cstring);
	}

	string::string(const char* str) {
		_length = strlen(str);
		_size = _length + 1;
		_cstring = new char[_size];
		strcpy(_cstring, str);
	}

	string::string(const char* str, size_t length): _length(length + 1) {
		_size = _length + 1;
		_cstring = new char[_size];
		_cstring[_length - 1] = '\0';
		memcpy(_cstring, str, _length - 1);
	}

	string::~string(){
		delete[] _cstring;
		_cstring = nullptr;
	}

	string& string::operator=(const string& str) {
		if (this != &str) {
			_length = str._length;
			_size = str._size;
			delete[] _cstring;
			_cstring = new char[this->_size];

			strcpy(_cstring, str._cstring);
		}

		return *this;
	}

	string& string::operator+=(const string& str) {
		_length = _length + str._length;
		_size = _length + 1;

		char* buffer = new char[_size];
		strcpy(buffer, _cstring);
		strcat(buffer, str._cstring);

		delete[] _cstring;
		_cstring = buffer;

		return *this;
	}

	string string::operator+(const string& str) const {
		char* buffer = new char[_length + str._length + 1];
		strcpy(buffer, _cstring);
		strcat(buffer, str._cstring);

		string ret = string(buffer);
		delete[] buffer;

		return ret;
	}

	string& string::operator=(const char* str) {
		delete[] _cstring;
		_length = strlen(str);
		_size = _length + 1;
		_cstring = new char[_size];
		strcpy(_cstring, str);
		return *this;
	}

	bool string::operator==(const string &str) const {
		return strcmp(_cstring, str._cstring);
	}

	bool string::operator==(const char *str) const {
		return strcmp(str, _cstring);
	}

	bool string::operator!=(const string &str) const {
		return !strcmp(_cstring, str._cstring);
	}

	bool string::operator!=(const char *str) const {
		return !strcmp(str, _cstring);
	}

	char& string::operator[](size_t index) const {
		return _cstring[index];
	}

	size_t string::length() const {
		return _length;
	}

	string string::substr(size_t start, size_t length) const {
		char* tmp = new char[length + 1];
		memcpy(tmp, _cstring + start, length);
		tmp[length] = '\0';

		string ret(tmp);
		delete[] tmp;

		return ret;
	}

	size_t string::find(const string& str, size_t start) const {
		return find(str._cstring, start);
	}

	size_t string::find(const char *str, size_t start) const {
		size_t len = strlen(str);
		for(auto i = start; i <= _length - len; i++) {
			if(operator[](i) == str[0]) {
				for(size_t j = 0; j < len; j++) {
					if(operator[](i + j) != str[j]) break;
				}
				return i;
			}
		}
		return -1;
	}

	size_t string::find(const char c, size_t start) const {
		for(auto i = start; i <= _length; i++) {
			if(operator[](i) == c) return i;
		}
		return -1;
	}

	size_t string::find_last_of(const string& str, size_t end) const {
		return find_last_of(str.c_str(), end);
	}

	size_t string::find_last_of(const char* str, size_t end) const {
		size_t find_len = strlen(str);
		if(!find_len) return -1;
		if(end == -1) end = _length - 1;
		end -= find_len;
		for(auto i = end; i > 0; i--) {
			if(operator[](i - 1) == str[0]) {
				for(size_t j = 0; j < find_len; j++) {
					if(operator[](i + j) != str[j]) break;
				}
				return i;
			}
		}
		return -1;
	}

	size_t string::find_last_of(const char c, size_t end) const {
		if(end == -1) end = _length - 1;
		end++;
		for(auto i = end; i > 0; i--) {
			if(operator[](i - 1) == c) return i;
		}
		return -1;
	}

	char *string::c_str() const {
		return _cstring;
	}

	char *string::data() const {
		return _cstring;
	}
}