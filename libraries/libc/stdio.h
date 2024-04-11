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

#ifndef DUCKOS_LIBC_STDIO_H
#define DUCKOS_LIBC_STDIO_H

// We have to add this to get GCC to build for duckOS for some reason. May be fixed with a newer version of GCC.
#ifdef GCC_SYSTEM_H
#define HAVE_TERMIOS_H
#endif

#include <sys/cdefs.h>
#include <stddef.h>
#include <stdarg.h>

__DECL_BEGIN

#define _STDIO_H // We need this so gmp knows we have a stdio.h header :)

#define FILENAME_MAX 255
#define BUFSIZ 8192

#include <kernel/api/unistd.h>
#include <kernel/api/types.h>

#define _IOFBF 0
#define _IOLBF 1
#define _IONBF 2

#ifndef EOF
#define EOF (-1)
#endif

typedef struct FILE FILE;
typedef long fpos_t;

extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

//File stuff
int remove(const char* filename);
int rename(const char* oldname, const char* newname);
FILE* tmpfile();
char* tmpnam(char* s);
int fclose(FILE* stream);
int fflush(FILE* stream);
FILE* fopen(const char* filename, const char* mode);
FILE* fdopen(int fd, const char* mode);
FILE* freopen(const char* filename, const char* mode, FILE* stream);
void setbuf(FILE* stream, char* buf);
int setvbuf(FILE* stream, char* buf, int mode, size_t size);
int fileno(FILE* stream);

//Formatted input/output
int fprintf(FILE* stream, const char* format, ...);
int fscanf(FILE* stream, const char* format, ...);
int printf(const char* format, ...);
int scanf(const char* format, ...);
int snprintf(char* s, size_t n, const char* format, ...);
int sprintf(char* s, const char* format, ...);
int sscanf(const char* s, const char* format, ...);
int vfprintf(FILE* stream, const char* format, va_list arg);
int vfscanf(FILE* stream, const char* format, va_list arg);
int vprintf(const char* format, va_list arg);
int vscanf(const char* format, va_list arg);
int vsprintf(char* s, const char* format, va_list arg);
int vsscanf(const char* s, const char* format, va_list arg);
int vasprintf(char** s, const char* format, va_list arg);
int vsnprintf(char* s, size_t n, const char* format, va_list arg);

//Character input/output
int fgetc(FILE* stream);
char* fgets(char* s, int n, FILE* stream);
int fputc(int c, FILE* stream);
int fputs(const char* s, FILE* stream);
int getc(FILE* stream);
int getchar();
int putc(int c, FILE* stream);
int putchar(int c);
int puts(const char* s);
int ungetc(int c, FILE* stream);

//Direct input/output
size_t fread(void* ptr, size_t size, size_t count, FILE* stream);
size_t fwrite(const void* ptr, size_t size, size_t count, FILE* stream);

//File positioning
int fgetpos(FILE* stream, fpos_t* pos);
int fseek(FILE* stream, long int offset, int whence);
int fseeko(FILE* stream, off_t offset, int whence);
int fsetpos(FILE* stream, const fpos_t* pos);
long int ftell(FILE* stream);
off_t ftello(FILE* stream);
void rewind(FILE* stream);

//Error handling
void clearerr(FILE* stream);
int feof(FILE* stream);
int ferror(FILE* stream);
void perror(const char* s);

//Internal stuff
void __init_stdio();
void __cleanup_stdio();

__DECL_END

#endif //DUCKOS_LIBC_STDIO_H
