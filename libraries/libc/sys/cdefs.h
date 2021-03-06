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

#ifndef DUCKOS_LIBC_DECL_H
#define DUCKOS_LIBC_DECL_H

#ifdef __cplusplus
	#ifndef __DECL_BEGIN
		#define __DECL_BEGIN extern "C" {
		#define __DECL_END }
	#endif
#else
	#ifndef __DECL_BEGIN
		#define __DECL_BEGIN
		#define __DECL_END
	#endif
#endif

#endif //DUCKOS_LIBC_DECL_H
