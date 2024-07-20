/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "strerror.h"

char* strerror(int errnum) {
	switch(errnum) {
		case EPERM:
			return "Operation not permitted";
		case ENOENT:
			return "No such file or directory";
		case ESRCH:
			return "No such process";
		case EINTR:
			return "Interrupted system call";
		case EIO:
			return "I/O error";
		case ENXIO:
			return "No such device or address";
		case E2BIG:
			return "Arg list too long";
		case ENOEXEC:
			return "Exec format error";
		case EBADF:
			return "Bad file number";
		case ECHILD:
			return "No child process";
		case EAGAIN:
			return "Try again";
		case ENOMEM:
			return "Out of memory";
		case EACCES:
			return "Permission denied";
		case EFAULT:
			return "Bad address";
		case EBUSY:
			return "Device or resource busy";
		case EEXIST:
			return "File exists";
		case EXDEV:
			return "Cross device link";
		case ENODEV:
			return "No such device";
		case ENOTDIR:
			return "Not a directory";
		case EISDIR:
			return "Is a directory";
		case EINVAL:
			return "Invalid argument";
		case ENFILE:
			return "File table overflow";
		case EMFILE:
			return "Too many open files";
		case ENOTTY:
			return "Not a typewriter";
		case ETXTBSY:
			return "Text file busy";
		case EFBIG:
			return "File too large";
		case ENOSPC:
			return "No space left on device";
		case ESPIPE:
			return "Illegal seek";
		case EROFS:
			return "Read-only file system";
		case EMLINK:
			return "Too many links";
		case EPIPE:
			return "Broken pipe";
		case EDOM:
			return "Math argument out of domain of func";
		case ERANGE:
			return "Math result not representable";
		case EDEADLK:
			return "Resource deadlock would occur";
		case ENAMETOOLONG:
			return "File name too long";
		case ENOLCK:
			return "No record locks available";
		case ENOSYS:
			return "Function not implemented";
		case ENOTEMPTY:
			return "Directory not empty";
		case ELOOP:
			return "Too many symbolic links";
		case ENOMSG:
			return "No message of desired type";
		case EIDRM:
			return "Identifier removed";
		case EADDRINUSE:
			return "Address in use";
		case ENOTSOCK:
			return "File descriptor is not a socket";
		case EHOSTUNREACH:
			return "Host is unreachable";
		case EHOSTDOWN:
			return "Host is down";
		case EADDRNOTAVAIL:
			return "Address not available";
		case EAFNOSUPPORT:
			return "Address family not supported";
		case ECONNABORTED:
			return "Connection aborted";
		case ECONNREFUSED:
			return "Connection refused";
		case ECONNRESET:
			return "Connection reset";
		case ENETUNREACH:
			return "Network unreachable";
		case ENETDOWN:
			return "Network interface is not configured";
		case ETIMEDOUT:
			return "Connection timed out";
		default:
			return "Unknown error";
	}
}