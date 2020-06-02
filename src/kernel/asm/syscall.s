[bits 32]
[extern syscallHandler]
global syscall_handler
syscall_handler:
	push ebx
	push eax
	call syscallHandler
	pop eax
	pop ebx
	iret
