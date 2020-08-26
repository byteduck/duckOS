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

struct FILE {
	int fd;
	char* read_buf;
	int available;
	off_t offset;
};

FILE* __stdin = NULL;
FILE* __stdout = NULL;
FILE* __stderr = NULL;

//File stuff
int remove(const char* filename) {
	return -1;
}

int rename(const char* old, const char* new) {
	return -1;
}

FILE* tmpfile() {
	return NULL;
}

char* tmpnam(char* s) {
	return NULL;
}

int fclose(FILE* stream) {
	return -1;
}

int fflush(FILE* stream) {
	return -1;
}

FILE* fopen(const char* restrict filename, const char* restrict mode) {
	return NULL;
}

FILE* freopen(const char* restrict filename, const char* restrict mode, FILE* restrict stream) {
	return NULL;
}

void setbuf(FILE* restrict stream, char* restrict buf) {

}

int setvbuf(FILE* restrict stream, char* restrict buf, int mode, size_t size) {
	return -1;
}

//Formatted input/output
int fprintf(FILE* restrict stream, const char* restrict format, ...) {
	return -1;
}

int fscanf(FILE* restrict stream, const char* restrict format, ...) {
	return -1;
}

int printf(const char* restrict format, ...) {
	return -1;
}

int scanf(const char* restrict format, ...) {
	return -1;
}

int snprintf(char* restrict s, size_t n, const char* restrict format, ...) {
	return -1;
}

int sprintf(char* restrict s, const char* restrict format, ...) {
	return -1;
}

int sscanf(const char* restrict s, const char* restrict format, ...) {
	return -1;
}

int vfprintf(FILE* restrict stream, const char* restrict format, va_list arg) {
	return -1;
}

int vfscanf(FILE* restrict stream, const char* restrict format, va_list arg) {
	return -1;
}

int vprintf(const char* restrict format, va_list arg) {
	return -1;
}

int vscanf(const char* restrict format, va_list arg) {
	return -1;
}

int vsnprintf(char* restrict s, size_t n, const char* restrict format, va_list arg) {
	return -1;
}

int vsprintf(char* restrict s, const char* restrict format, va_list arg) {
	return -1;
}

int vsscanf(const char* restrict s, const char* restrict format, va_list arg) {
	return -1;
}

//Character input/output
int fgetc(FILE* stream) {
	return -1;
}

char* fgets(char* restrict s, int n, FILE* restrict stream) {
	return NULL;
}

int fputc(int c, FILE* stream) {
	return -1;
}

int fputs(const char* restrict s, FILE* restrict stream) {
	return -1;
}

int getc(FILE* stream) {
	return -1;
}

int getchar() {
	return -1;
}

int putc(int c, FILE* stream) {
	return -1;
}

int putchar(int c) {
	return -1;
}

int puts(const char* s) {
	return -1;
}

int ungetc(int c, FILE* stream) {
	return -1;
}

//Direct input/output
size_t fread(void* restrict ptr, size_t size, size_t nmemb, FILE* restrict stream) {
	return 0;
}

size_t fwrite(const void* restrict ptr, size_t size, size_t nmemb, FILE* restrict stream) {
	return 0;
}

//File positioning
int fgetpos(FILE* restrict stream, fpos_t* restrict pos) {
	return -1;
}

int fseek(FILE* stream, long int offset, int whence) {
	return -1;
}

int fsetpos(FILE* stream, const fpos_t* pos) {
	return -1;
}

long int ftell(FILE* stream) {
	return -1;
}

void rewind(FILE* stream) {

}

//Error handling
void clearerr(FILE* stream) {

}

int feof(FILE* stream) {
	return -1;
}

int ferror(FILE* stream) {
	return -1;
}

void perror(const char* s) {

}