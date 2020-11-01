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
#include <libc/sys/printf.h>

struct FILE {
	int fd;
	int options;
	int ungetc;
	int eof;
	ssize_t err;
	char* buffer;
	uint8_t bufmode; //The buffer mode (_IOFBF, _IOLBF, _IONBF)
	size_t offset; //The current offset into the buffer
	size_t bufavail; //The amount of bytes from the beginning of the buffer that can be read
	size_t bufsiz; //The size of the buffer
	FILE* next; //The next file in the linked list of all open files
	FILE* prev; //The previous file in the linked list of all open files
};

bool can_read(FILE* file) {
	return file->options & O_RDWR || !(file->options & O_WRONLY);
}

bool can_write(FILE* file) {
	return file->options & O_RDWR || file->options & O_WRONLY;
}

FILE __stdin = {
		.fd = STDIN_FILENO,
		.options = O_RDONLY,
		.ungetc = -1,
		.eof = 0,
		.err = 0,
		.buffer = NULL,
		.bufmode = _IONBF,
		.offset = 0,
		.bufavail = 0,
		.bufsiz = 0,
		.next = &__stdout,
		.prev = NULL
};

FILE __stdout = {
		.fd = STDOUT_FILENO,
		.options = O_WRONLY,
		.ungetc = -1,
		.eof = 0,
		.err = 0,
		.buffer = NULL,
		.bufmode = _IONBF,
		.offset = 0,
		.bufavail = 0,
		.bufsiz = 0,
		.next = &__stderr,
		.prev = &__stdin
};

FILE __stderr = {
		.fd = STDERR_FILENO,
		.options = O_WRONLY,
		.ungetc = -1,
		.eof = 0,
		.err = 0,
		.buffer = NULL,
		.bufmode = _IONBF,
		.offset = 0,
		.bufavail = 0,
		.bufsiz = 0,
		.next = NULL,
		.prev = &__stdout
};

//File stuff
FILE* filelist_first = &__stdin;
FILE* filelist_last = &__stderr;

void filelist_insert(FILE* file) {
	filelist_last->next = file;
	filelist_last = file;
}

void filelist_remove(FILE* file) {
	if(file->next)
		file->next->prev = file->prev;
	if(file->prev)
		file->prev->next = file->next;
	if(file == filelist_last)
		filelist_last = file->prev;
}

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
	int flush_result = fflush(stream);
	int close_result = close(stream->fd);
	if(flush_result < 0)
		errno = stream->err;
	filelist_remove(stream);
	return !flush_result && !close_result ? 0 : -1;
}

int fflush(FILE* stream) {
	if(stream->bufmode == _IONBF)
		return 0;

	if(can_write(stream) && !stream->bufavail) {
		//Flush the data written to the buffer
		int res = write(stream->fd, stream->buffer, stream->offset);
		if(res < 0)
			stream->err = errno;
	}

	if(can_read(stream) && stream->bufavail) {
		//Seek back to where we were in the read buffer so it's where the user expects
		int res = lseek(stream->fd, -(stream->bufavail - stream->offset), SEEK_CUR);
		if(res < 0) {
			//Can't seek pipes, we don't care
			if(errno == ESPIPE) {
				errno = 0;
				return 0;
			}

			stream->err = errno;
			return -1;
		}
	}

	//Clear the buffer
	stream->offset = 0;
	stream->bufavail = 0;

	return 0;
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
	ret->options = options;
	ret->ungetc = -1;
	ret->eof = 0;
	ret->err = 0;
	ret->bufsiz = 0;
	ret->buffer = NULL;
	ret->bufavail = 0;
	ret->offset = 0;
	setvbuf(ret, NULL, _IOFBF, BUFSIZ);
	filelist_insert(ret);

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
	ret->options = options;
	ret->ungetc = -1;
	ret->eof = 0;
	ret->err = 0;
	ret->bufsiz = 0;
	ret->buffer = NULL;
	ret->bufavail = 0;
	ret->offset = 0;
	setvbuf(ret, NULL, _IOFBF, BUFSIZ);

	return 0;
}

FILE* freopen(const char* filename, const char* mode, FILE* stream) {
	stream->err = 0;
	return NULL;
}

void setbuf(FILE* stream, char* buf) {
	setvbuf(stream, buf, buf ? _IOFBF : _IONBF, BUFSIZ);
}

int setvbuf(FILE* stream, char* buf, int mode, size_t size) {
	if(mode != _IOFBF && mode != _IOLBF && mode != _IONBF)
		return -1;

	//Free the previous buffer if there is one
	fflush(stream);
	if(stream->buffer)
		free(stream->buffer);

	//If the mode is _IONBF, don't set a buffer
	stream->bufmode = mode;
	if(mode == _IONBF) {
		stream->buffer = NULL;
		stream->bufsiz = 0;
		return 0;
	}

	//Allocate or set the buffer
	if(buf)
		stream->buffer = buf;
	else
		stream->buffer = malloc(size);
	stream->bufsiz = size;

	return 0;
}

int fileno(FILE* stream) {
	return stream->fd;
}

//Formatted input/output
int fprintf(FILE* stream, const char* format, ...) {
	va_list arg;
	int ret;
	va_start (arg, format);
	ret = vfprintf (stream, format, arg);
	va_end (arg);
	return ret;
}

int fscanf(FILE* stream, const char* format, ...) {
	return -1;
}

int printf(const char* format, ...) {
	va_list arg;
	va_start(arg, format);
	int ret = vfprintf(stdout, format, arg);
	va_end(arg);
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

int vsnprintf(char* s, size_t n, const char* format, va_list arg) {
	return common_printf(s, n, format, arg);
}

//Character input/output
int fgetc(FILE* stream) {
	char buf[1];
	fread(buf, 1, 1, stream);
	return (unsigned char) *buf;
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
	fwrite(s, 1, strlen(s), stdout);
	fwrite("\n", 1, 1, stdout);
	return 0;
}

int ungetc(int c, FILE* stream) {
	if(stream->ungetc != -1)
		return EOF;

	stream->ungetc = c;
	return c;
}

//Direct input/output
size_t fread(void* ptr, size_t size, size_t count, FILE* stream) {
	if(!count || !size)
		return 0;

	char* buf = (char*) ptr;
	size_t len = count * size;
	size_t nread = 0;

	//If we have something in ungetc, first account for that
	if(stream->ungetc >= 0) {
		*(buf++) = stream->ungetc;
		stream->ungetc = -1;
		nread++;
		len--;
	}

	//If we don't have a buffer, just read directly
	if(stream->bufmode == _IONBF) {
		//Read directly from the file
		ssize_t res = read(stream->fd, buf, len);
		if(res < 0)
			stream->err = res;
		else if(res < len)
			stream->eof = 1;
		else
			nread += res;

		return nread / size;
	}

	while(len) {
		size_t bufleft = stream->bufavail - stream->offset;
		size_t nbuf = bufleft < len ? bufleft : len;

		//If there's nothing left in the buffer, read into it
		if(!nbuf) {
			//If we previously reached eof, break
			if(stream->eof)
				break;
			fflush(stream);
			ssize_t res = read(stream->fd, stream->buffer, stream->bufsiz);
			if(res < 0) {
				stream->err = errno;
				break;
			} else if(res == 0)
				stream->eof = 1;
			stream->bufavail = res;
			continue;
		}

		//Copy from the buffer
		memcpy(buf, stream->buffer + stream->offset, nbuf);
		buf += nbuf;
		nread += nbuf;
		len -= nbuf;
		stream->offset += nbuf;
	}

	return nread / size;
}

size_t fwrite(const void* ptr, size_t size, size_t count, FILE* stream) {
	//If we're in no buffer mode, write directly
	if(stream->bufmode == _IONBF) {
		int res = write(stream->fd, ptr, count * size);
		if(res < 0) {
			stream->err = errno;
			return 0;
		}
		return res;
	}

	const char* buf = (const char*) ptr;
	size_t len = count * size;
	size_t nwrote = 0;

	while(len) {
		size_t bufleft = stream->bufsiz - stream->offset;
		size_t nbuf = bufleft < len ? bufleft : len;

		//If there's no space left in the buffer or we had previously used it for reading, flush
		if(!nbuf || stream->bufavail) {
			fflush(stream);
			continue;
		}

		//Copy to the buffer
		memcpy(stream->buffer + stream->offset, buf, nbuf);
		nwrote += nbuf;
		len -= nbuf;
		stream->offset += nbuf;
		stream->bufavail = 0;

		//If we are in line buffered mode and we wrote a newline, flush
		if(stream->bufmode == _IOLBF && memchr(buf, '\n', nwrote))
			fflush(stream);

		buf += nbuf;
	}

	return nwrote / size;
}

//File positioning
int fgetpos(FILE* stream, fpos_t* pos) {
	long int tell = ftell(stream);
	if(tell == EOF) return EOF;
	*pos = tell;
	return 0;
}

int fseek(FILE* stream, long int offset, int whence) {
	fflush(stream);
	stream->eof = 0;
	stream->ungetc = -1;
	return lseek(stream->fd, offset, whence);
}

int fsetpos(FILE* stream, const fpos_t* pos) {
	return fseek(stream, *pos, SEEK_SET);
}

long int ftell(FILE* stream) {
	if(fflush(stream) < 0)
		return -1;
	return lseek(stream->fd, 0, SEEK_CUR);
}

void rewind(FILE* stream) {
	fseek(stream, 0, SEEK_SET);
}

//Error handling
void clearerr(FILE* stream) {
	stream->eof = 0;
	stream->err = 0;
}

int feof(FILE* stream) {
	return stream->eof;
}

int ferror(FILE* stream) {
	return stream->err;
}

void perror(const char* s) {
	fprintf(stderr, "%s: %s\n", s, strerror(errno));
}

//Internal
void __init_stdio() {
	setvbuf(&__stdin, NULL, _IOLBF, BUFSIZ);
	setvbuf(&__stdout, NULL, _IOLBF, BUFSIZ);
	setvbuf(&__stderr, NULL, _IOLBF, BUFSIZ);
}

void __cleanup_stdio() {
	FILE* cur = filelist_first;
	while(cur) {
		fclose(cur);
		cur = cur->next;
	}
}