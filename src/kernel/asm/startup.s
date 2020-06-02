[bits 32]
[global start]
[global load_gdt]
[extern kmain]
[global BootPageDirectory]

global flagss

KERNEL_VIRTUAL_BASE equ 0xC0000000                  ; 3GiB
KERNEL_PAGE_NUMBER equ (KERNEL_VIRTUAL_BASE >> 22)

section .data
align 0x1000
BootPageDirectory:                              ;The page directory for mapping the higher-half.
    dd 0x00000083
    times (KERNEL_PAGE_NUMBER - 1) dd 0
    dd 0x00000083                               ;Our kernel's page
    times (1024 - KERNEL_PAGE_NUMBER - 1) dd 0

retfromkernel: db 0xa,"|-----------------------|",0xa,"| Returned from kernel. |",0xa,"|-----------------------|" ;The message that shows when (if?) the kernel exits.

section .text
align 4
mboot:
	dd  0xe85250d6
	dd  0
	dd  mboot_end-mboot
	dd  -(0xe85250d6 + 0 + (mboot_end - mboot))
	dw 0
	dw 0
	dd 8
mboot_end:

start:
    mov ecx, (BootPageDirectory - KERNEL_VIRTUAL_BASE) ;We're subtracting KERNEL_VIRTUAL_BASE because this whole thing is compiled with an offset of 0xc0000000, and we want physical addresses.
    mov cr3, ecx

    mov ecx, cr4
    or ecx, 0x00000010 ;Enable PAE
    mov cr4, ecx

    mov ecx, cr0
    or ecx, 0x80000000 ;Turn on paging
    mov cr0, ecx

    lea ecx, [start_hh]
    jmp ecx

start_hh:
	mov dword [BootPageDirectory], 0
    	invlpg [0]

	mov esp, stack+0x4000
	push dword ebx

	call kmain

	mov eax, 1
	mov ebx, retfromkernel
	int 0x80
	jmp $

;%include 'asm/vesa.asm' I still gotta figure out how to deal with this in 32-bit mode
%include 'asm/gdt.s'
%include 'asm/syscall.s'
%include 'asm/int.s'
%include 'asm/paging.s'

section .bss
align 32
stack:
    resb 0x4000      ; reserve 16k stack on a uint64_t boundary
