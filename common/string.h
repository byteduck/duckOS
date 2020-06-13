#ifndef DUCKOS_STRING_H
#define DUCKOS_STRING_H

#include <common/cstddef.h>
#include <common/cstring.h>

namespace DC {
	class string {
	public:
		string();
		string(const string& string);
		string(const char* string);
		~string();

		string& operator=(const char* str);
		string& operator=(const string& str);
		string& operator+=(const string& str);
		string operator+(const string& b) const;
		bool operator==(const string& str) const;
		bool operator==(const char* str) const;
		bool operator!=(const string& str) const;
		bool operator!=(const char* str) const;
		char& operator[](size_t index) const;

		size_t length() const;
		char* c_str() const;
		char* data() const;
		string substr(size_t start, size_t length) const;
		size_t find(const string& str, size_t start = 0) const;
		size_t find(const char *str, size_t start = 0) const;
		size_t find(const char c, size_t start = 0) const;
	private:
		size_t _size;
		size_t _length;
		char* _cstring;
	};
}

#endif //DUCKOS_STRING_H
