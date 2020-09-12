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

#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <stdbool.h>

struct FILE {
	int fd;
	int options;
	off_t offset;
	int ungetc;
	int eof;
};

FILE __stdin = {
		.fd = STDIN_FILENO,
		.options = O_RDONLY,
		.offset = 0,
		.ungetc = -1,
		.eof = 0
};

FILE __stdout = {
		.fd = STDOUT_FILENO,
		.options = O_WRONLY,
		.offset = 0,
		.ungetc = -1,
		.eof = 0
};

FILE __stderr = {
		.fd = STDERR_FILENO,
		.options = O_WRONLY,
		.offset = 0,
		.ungetc = -1,
		.eof = 0
};

//File stuff
int remove(const char* filename) {
	return syscall2(SYS_UNLINK, (int) filename);
}

int rename(const char* oldname, const char* newname) {
	if(syscall3(SYS_LINK, (int) oldname, (int) newname)) return -1;
	return syscall2(SYS_UNLINK, (int) oldname);
}

FILE* tmpfile() {
	//TODO
	return NULL;
}

char* tmpnam(char* s) {
	//TODO
	return NULL;
}

int fclose(FILE* stream) {
	return close(stream->fd);
}

int fflush(FILE* stream) {
	//TODO
	return -1;
}

int parse_str_options(const char* mode) {
	int options;
	switch(mode[0]) {
		case 'r':
			options = O_RDONLY;
			break;
		case 'w':
			options = O_WRONLY | O_CREAT;
			break;
		case 'a':
			options = O_WRONLY | O_APPEND;
			break;
		default:
			return -1;
	}
	if(mode[1] == '+') {
		switch(options) {
			case O_RDONLY:
				options = O_RDWR;
				break;
			case O_WRONLY | O_CREAT:
				options = O_RDWR | O_CREAT;
				break;
			case O_WRONLY | O_APPEND:
				options = O_RDWR | O_APPEND;
				break;
			default: //Should never happen
				return -1;
		}
	}
	return options;
}

FILE* fopen(const char* filename, const char* mode) {
	//Parse options
	int options = parse_str_options(mode);
	if(options == -1) {
		errno = EINVAL;
		return NULL;
	}

	//Truncate if necessary
	if(options == (O_WRONLY | O_CREAT) || options == (O_RDWR | O_CREAT))
		truncate(filename, 0);

	//Open fd
	int fd = open(filename, options);
	if(fd == -1)
		return NULL;

	//Make the file
	FILE* ret = malloc(sizeof(FILE));
	ret->fd = fd;
	ret->offset = 0;
	ret->options = options;
	ret->ungetc = -1;
	ret->eof = 0;

	return ret;
}

FILE* fdopen(int fd, const char* mode) {
	//Parse options
	int options = parse_str_options(mode);
	if(options == -1) {
		errno = EINVAL;
		return NULL;
	}

	//Make the file
	FILE* ret = malloc(sizeof(FILE));
	ret->fd = fd;
	ret->offset = -1;
	ret->options = options;
	ret->ungetc = -1;
	ret->eof = 0;

	return 0;
}

FILE* freopen(const char* filename, const char* mode, FILE* stream) {
	return NULL;
}

void setbuf(FILE* stream, char* buf) {

}

int setvbuf(FILE* stream, char* buf, int mode, size_t size) {
	return -1;
}

int fileno(FILE* stream) {
	return stream->fd;
}

//Formatted input/output
int fprintf(FILE* stream, const char* format, ...) {
	va_list arg;
	int done;
	va_start (arg, format);
	done = vfprintf (stdout, format, arg);
	va_end (arg);
	return done;
}

int fscanf(FILE* stream, const char* format, ...) {
	return -1;
}

int printf(const char* format, ...) {
	va_list args;
	va_start(args, format);
	int ret = vfprintf(stdout, format, args);
	va_end(args);
	return ret;
}

int scanf(const char* format, ...) {
	va_list args;
	va_start(args, format);
	int ret = fscanf(stdin, format, args);
	va_end(args);
	return ret;
}

int snprintf(char* s, size_t n, const char* format, ...) {
	return -1;
}

int sprintf(char* s, const char* format, ...) {
	return -1;
}

int sscanf(const char* s, const char* format, ...) {
	return -1;
}

int vfprintf(FILE* stream, const char* format, va_list arg) {
	char* str;
	int ret = vasprintf(&str, format, arg);
	fwrite(str, ret, 1, stream);
	free(str);
	return ret;
}

int vfscanf(FILE* stream, const char* format, va_list arg) {
	return -1;
}

int vprintf(const char* format, va_list arg) {
	return vfprintf(stdout, format, arg);
}

int vscanf(const char* format, va_list arg) {
	return vfscanf(stdin, format, arg);
}

int vsprintf(char* s, const char* format, va_list arg) {
	return -1;
}

int vsscanf(const char* s, const char* format, va_list arg) {
	return -1;
}

int vasprintf(char** s, const char* format, va_list arg) {
	size_t len = strlen(format) + 100;
	*s = malloc(len);
	return vsnprintf(*s, len, format, arg);
}

#define INTERPRET_NUMBER(s, n) \
	while(*(s) >= '0' && *(s) <= '9') { \
		(n) *= 10; \
		(n) += *(s) - '0'; \
		(s)++; \
	}

void hex_str(unsigned int val, char** buf, unsigned int width, size_t n, size_t* len, bool upper) {
	//Figure out the width of the resulting string
	size_t num_chars = 0;
	unsigned int val2 = val;
	while(val2) {
		num_chars ++;
		val2 /= 0x10;
	}

	//Zero pad
	for(size_t i = num_chars; i < width && *len < n; i++) {
		**buf = '0';
		(*buf)++;
		(*len)++;
	}

	//Print the hex
	char* hex = upper ? "0123456789ABCDEF" : "0123456789abcdef";
	for(size_t i = 0; i < num_chars && *len < n; i++) {
		**buf = hex[(val >> (i * 4)) & 0xFu];
		(*buf)++;
		(*len)++;
	}
}

void dec_str(unsigned int val, char** buf, unsigned int width, size_t n, size_t* len) {
	//Figure out the width of the resulting string
	size_t num_chars = 0;
	unsigned int val2 = val;
	while(val2) {
		num_chars ++;
		val2 /= 10;
	}

	//Zero pad
	for(size_t i = num_chars; i < width && *len < n; i++) {
		**buf = '0';
		(*buf)++;
		(*len)++;
	}

	//Print the decimal
	for(size_t i = 0; i < num_chars && *len < n; i++) {
		**buf = "0123456789"[val % 10];
		val /= 10;
		(*buf)++;
		(*len)++;
	}
}

int vsnprintf(char* s, size_t n, const char* format, va_list arg) {
	char* buf = s;
	int precision = -1;
	size_t len = 0;
	for(const char* p = format; *p && len < n; p++) {
		//If it's not a percent, add it to the buffer
		if(*p != '%') {
			*buf++ = *p;
			len++;
			continue;
		}

		p++;
		bool zero_pad = false;
		bool force_sign = false;
		bool left_justify = false;
		bool alt_prefix = false;
		bool space_no_sign = false;
		uint8_t size = 0;
		unsigned int arg_width = 0;

		//Interpret arguments to the format
		while(1) {
			char ch = *p;
			if(ch == '-') {
				left_justify = true;
				p++;
			} else if(ch == '+') {
				force_sign = true;
				p++;
			} else if(ch == '0') {
				zero_pad = true;
				p++;
			} else if(ch == '*') {
				arg_width = (char) va_arg(arg, int);
				p++;
			} else if(ch == '$') {
				alt_prefix = true;
				p++;
			} else if(ch == ' ') {
				space_no_sign = true;
				p++;
			} else break;
		}

		//Interpret the argument width
		INTERPRET_NUMBER(p, arg_width);

		//Interpret argument precision
		if(*p == '.') {
			p++;
			precision = 0;
			if(*p == '*') {
				//Precision is given in argument list
				precision = (int) va_arg(arg, int);
				p++;
			} else {
				//Precision is given in string
				INTERPRET_NUMBER(p, precision);
			}
		}

		//Interpret number size
		if(*p == 'z') {
			size = 1;
			p++;
		} else if(*p == 'l') {
			p++;
			size = 1;
			if(*p == 'l') {
				p++;
				size = 2;
			}
		}

		bool uppercase = false;
		switch(*p) {
			case 'c': //Character
				*buf++ = (char) va_arg(arg, int);
				break;

			case 'p': //Pointer address
				if(!arg_width)
					arg_width = 8;
				goto hex;
			case 'X':
				uppercase = true;
			case 'x': //Hex
			hex:
				if(alt_prefix) {
					*buf++ = '0';
					*buf++ = 'x';
					len += 2;
				}
				if(size == 2) { //long long
					unsigned long long val = (unsigned long long) va_arg(arg, unsigned long long);
					if(val > 0xFFFFFFFF)
						hex_str(val >> 32, &buf, arg_width > 8 ? (arg_width - 8) : 0, n, &len, uppercase);
					hex_str(val & 0xFFFFFFFF, &buf, arg_width > 8 ? 8 : arg_width, n, &len, uppercase);
				} else { //unsigned long or smaller
					unsigned long val = va_arg(arg, unsigned long);
					hex_str(val, &buf, arg_width, n, &len, uppercase);
				}
				break;

			case 'i':
			case 'd': //Dec
			{
				long long val;
				if(size == 2)
					val = (long long) va_arg(arg, long long);
				else
					val = (long long) va_arg(arg, long);

				//Print sign if necessary
				if(val < 0) {
					*buf++ = '-';
					val = -val;
					len++;
				} else if(force_sign) {
					*buf++ = '+';
					len++;
				} else if(space_no_sign) {
					*buf++ = ' ';
					len++;
				}

				dec_str(val, &buf, arg_width, n, &len);
				break;
			}

			case 'u': //Unsigned dec
			{
				unsigned long long val;
				if(size == 2)
					val = (unsigned long long) va_arg(arg, unsigned long long);
				else
					val = (unsigned long long) va_arg(arg, unsigned long);

				//Print sign if necessary
				if(force_sign) {
					*buf++ = '+';
					len++;
				} else if(space_no_sign) {
					*buf++ = ' ';
					len++;
				}

				dec_str(val, &buf, arg_width, n, &len);
				break;
			}

			case 'F':
				uppercase = true;
			case 'f': //Float
				//TODO
				break;

			case 's': //String
			{
				char* str = (char*) va_arg(arg, char*);
				if(!str) str = "(null)";
				size_t nchr = 0;
				//Precision and arg width are used as string length
				if(precision < 0) {
					while(*str && len < n) {
						*buf++ = *str++;
						nchr++;
						len++;
						if(arg_width && nchr == arg_width) break;
					}
				} else {
					while(*str && precision && len < n) {
						*buf++ = *str++;
						nchr++;
						precision--;
						len++;
						if(arg_width && nchr == arg_width) break;
					}
				}
				break;
			}

			case '%':
				*buf++ = '%';
				len++;
				break;

			default:
				//TODO: e, E, g, G, n
				*buf++ = *p;
				len++;
				break;
		}
	}

	*buf = '\0';
	return len;
}

//Character input/output
int fgetc(FILE* stream) {
	char buf[1];
	if(fread(buf, 1, 1, stream) <= 0) {
		stream->eof = 1;
		return EOF;
	}
	return *buf;
}

char* fgets(char* s, int n, FILE* stream) {
	int c;
	char* os = s;

	if(n == 1) {
		*s = '\0';
		return os;
	}

	while((c = fgetc(stream)) > 0) {
		*s++ = c;

		//If we've read n-1 characters, null-terminate the string and return it
		n--;
		if(n == 1) {
			*s = '\0';
			return os;
		}

		//Null-terminate the string at the current position and return it if we encounter a newline
		if(c == '\n') {
			*s = '\0';
			return os;
		}

		//If we encounter an error, return the string (or null if we haven't read anything)
		if(c == -1) {
			stream->eof = 1;
			return os == s ? NULL : os;
		}
	}

	return NULL;
}

int fputc(int c, FILE* stream) {
	if(fwrite(&c, 1, 1, stream) == -1) {
		stream->eof = 1;
		return EOF;
	}
	return c;
}

int fputs(const char* s, FILE* stream) {
	if(fwrite(s, 1, strlen(s), stream) == -1)
		return EOF;
	return 0;
}

int getc(FILE* stream) {
	return fgetc(stream);
}

int getchar() {
	return fgetc(stdin);
}

int putc(int c, FILE* stream) {
	return fputc(c, stdout);
}

int putchar(int c) {
	return fputc(c, stdout);
}

int puts(const char* s) {
	return fputs(s, stdout);
}

int ungetc(int c, FILE* stream) {
	if(stream->ungetc != -1)
		return EOF;

	stream->ungetc = c;
	return c;
}

//Direct input/output
size_t fread(void* ptr, size_t size, size_t count, FILE* stream) {
	if(count <= 0)
		return 0;

	//If we have something in ungetc, account for that
	if(stream->ungetc >= 0) {
		stream->ungetc = -1;
		((char*) ptr)[0] = stream->ungetc;

		ssize_t nread = read(stream->fd, ptr + 1, (count * size) - 1);
		if(nread == -1)
			return nread;
		return nread + 1;
	}

	//Otherwise, just read
	return read(stream->fd, ptr, count * size);
}

size_t fwrite(const void* ptr, size_t size, size_t count, FILE* stream) {
	return write(stream->fd, ptr, count * size);
}

//File positioning
int fgetpos(FILE* stream, fpos_t* pos) {
	long int tell = ftell(stream);
	if(tell == EOF) return EOF;
	*pos = tell;
	return 0;
}

int fseek(FILE* stream, long int offset, int whence) {
	return lseek(stream->fd, offset, whence);
}

int fsetpos(FILE* stream, const fpos_t* pos) {
	return fseek(stream, *pos, SEEK_SET);
}

long int ftell(FILE* stream) {
	return stream->offset; //TODO Handle if file was opened with fdopen
}

void rewind(FILE* stream) {
	fseek(stream, 0, SEEK_SET);
}

//Error handling
void clearerr(FILE* stream) {
	stream->eof = 0;
}

int feof(FILE* stream) {
	return stream->eof;
}

int ferror(FILE* stream) {
	return 0; //TODO
}

void perror(const char* s) {
	fprintf(stderr, "%s: %s", s, strerror(errno));
}

//Internal
void __init_stdio() {
	//TODO
}