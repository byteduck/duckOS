/*
	This file is part of duckOS.

	duckOS is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	duckOS is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY// without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with duckOS.  If not, see <https://www.gnu.org/licenses/>.

	Copyright (c) Byteduck 2016-2024. All rights reserved.
*/

.section .text


.global setjmp
.type setjmp, @function
.align 4
setjmp:
    xor %eax, %eax
    mov 4(%esp), %eax  // jmp_buf pointer
    mov %ebx, (%eax)   // jmp_buf[0] = %ebx
    mov %esi, 4(%eax)  // jmp_buf[1] = %esi
    mov %edi, 8(%eax)  // jmp_buf[2] = %edi
    mov %ebp, 12(%eax) // jmp_buf[3] = %ebp
    lea 4(%esp), %ecx
    mov %ecx, 16(%eax) // jmp_buf[4] = %esp (before call)
    mov (%esp), %ecx
    mov %ecx, 20(%eax) // jmp_buf[5] = %eip (caller-saved)
    xor %eax, %eax
    ret

.global longjmp
.type longjmp, @function
.align 4
longjmp:
    mov 4(%esp), %edx // jmp_buf
    mov 8(%esp), %eax // return val
    test %eax, %eax   // val == 0?
    jnz .valzero
    inc %eax
    .valzero:
    mov (%edx),%ebx   // %ebx = jmp_buf[0]
    mov 4(%edx),%esi  // %esi = jmp_buf[1]
    mov 8(%edx),%edi  // %edi = jmp_buf[2]
    mov 12(%edx),%ebp // %ebp = jmp_buf[3]
    mov 16(%edx),%ecx
    mov %ecx,%esp     // %esp = jmp_buf[4]
    mov 20(%edx),%ecx
    jmp *%ecx         // %eip = jmp_buf[5]