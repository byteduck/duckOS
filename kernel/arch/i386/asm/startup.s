[bits 32]
[global start]
[global load_gdt]
[extern i386init]

global flagss

KERNEL_VIRTUAL_BASE equ 0xC0000000                  ; 3GiB

section .pagetables
align 0x1000


;Page tables
IdentityPageTable:
    times 4096 dd 0
KernelPageTable1:
    times 4096 dd 0
KernelPageTable2:
    times 4096 dd 0

;Page directory
BootPageDirectory:
    times (KERNEL_VIRTUAL_BASE >> 22) dd 0
HigherHalfPDStart:
    times (1024 - (KERNEL_VIRTUAL_BASE >> 22)) dd 0

section .data
align 0x1000

;Variables
mbootptr:
    dd 0

section .multiboot
align 4
mboot:
	dd  0x1BADB002           ;Magic
	dd  0x7                  ;Flags (4KiB-aligned modules, memory info, framebuffer info)
	dd  -(0x1BADB002 + 0x7)  ;Checksum
	times 5 dd 0
	dd 0                      ;Graphics mode
	dd 640                    ;Graphics width
	dd 480                    ;Graphics height
	dd 32                     ;Graphics depth
mboot_end:

section .text

start:
    mov [mbootptr - KERNEL_VIRTUAL_BASE], ebx

    ;Identity map first 4MiB
    mov ebx, 0x0
    mov edx, IdentityPageTable - KERNEL_VIRTUAL_BASE
    call fill_table

    ;Assign identity entry in page directory
    mov ecx, (IdentityPageTable - KERNEL_VIRTUAL_BASE)
    or ecx, 3
    mov [BootPageDirectory - KERNEL_VIRTUAL_BASE], ecx

    ;Map first 8MiB of kernel
    mov eax, 0x0
    mov ebx, 0x0
    mov edx, (KernelPageTable1 - KERNEL_VIRTUAL_BASE)
    call fill_table
    mov eax, 0x0
    mov ebx, (1024 * 4096)
    mov edx, (KernelPageTable2 - KERNEL_VIRTUAL_BASE)
    call fill_table

    ;Assign kernel entries in page directory
    mov ecx, (KernelPageTable1 - KERNEL_VIRTUAL_BASE)
    or ecx, 3
    mov [HigherHalfPDStart - KERNEL_VIRTUAL_BASE], ecx
    mov ecx, (KernelPageTable2 - KERNEL_VIRTUAL_BASE)
    or ecx, 3
    mov [HigherHalfPDStart - KERNEL_VIRTUAL_BASE + 4], ecx

    ;Load boot page directory into cr3
    mov ecx, (BootPageDirectory - KERNEL_VIRTUAL_BASE)
    mov cr3, ecx

    ;Turn on paging
    mov ecx, cr0
    or ecx, 0x80010000
    mov cr0, ecx

    ;Check for SSE
    mov eax, 0x1
    cpuid
    test edx, 1<<25
    jz no_sse

    ;Turn on SSE
    mov eax, cr0
    and ax, 0xFFFB
    or ax, 0x2
    mov cr0, eax
    mov eax, cr4
    or ax, 3 << 9
    mov cr4, eax
    no_sse:

    lea ecx, [start_hh]
    jmp ecx

start_hh:
	mov dword [BootPageDirectory], 0
    	invlpg [0]

	mov esp, stack+0x4000
	push dword [mbootptr]

	call i386init

	jmp $

;Map a pagetable pointed to by edx starting at physical address ebx
fill_table:
    push eax
    mov eax, 0
fill_table_loop:
    mov ecx, ebx
    or ecx, 3
    mov [edx+eax*4], ecx
    add ebx, 4096
    inc eax
    cmp eax, 1024
    je fill_table_end
    jmp fill_table_loop
fill_table_end:
    pop eax
    ret

section .bss
align 32
[global stack]
stack:
    resb 0x4000      ; reserve 16k stack on a uint64_t boundary
[global heap]
heap:
    resb 0x
