[bits 32]
[global load_page_dir]
load_page_dir:
	push ebp           ;
	mov ebp, esp       ;Set the stack location to where we stored the passed variables in C
	push dword [ebp+8] ;
	pop eax            ;The page directory variable
	
	pusha
	mov cr3, eax

    ;Turn on paging in cr0 register
	mov eax, cr0
	or eax, 0x80000000
	mov cr0, eax
	popa
	
	mov esp, ebp ;
	pop ebp      ;Restore the stack
	ret