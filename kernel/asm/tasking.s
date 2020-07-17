global irq0
[extern pit_handler]
[extern preempt]
[extern tasking_enabled]

irq0
    push eax
    push ebx
    push ecx
    push edx
	call pit_handler
	pop edx
	pop ecx
	pop ebx
	mov eax, 0x20
	out 0x20, al
	pop eax
	iret

[global preempt_now_asm]
preempt_now_asm:
    cli
    push eax
    push ebx
    push ecx
    push edx
    call preempt
    pop edx
    pop ecx
    pop ebx
    mov eax, 0x20
    out 0x20, al
    pop eax
    ret

[global preempt_init_asm]
preempt_init_asm: ;Pretty much the same as preempt_asm, but without storing the state of the current process
    mov eax, [esp+4]
    mov esp, eax
    pop gs
    pop fs
    pop es
    pop ds
    pop esi
    pop edi
    pop ebp
    iret

[global preempt_asm]
preempt_asm:
    mov eax, [esp+4] ;old_esp
    mov ecx, [esp+8] ;new_esp
    mov edx, [esp+12] ;new_cr3
    push ebp
    push edi
    push esi
    push ds
    push es
    push fs
    push gs
    mov [eax], esp
    mov cr3, edx
    mov esp, [ecx]
    pop gs
    pop fs
    pop es
    pop ds
    pop esi
    pop edi
    pop ebp
    ret

[global proc_first_preempt]
proc_first_preempt:
    pop edx
    pop ecx
    pop ebx
	mov eax, 0x20
	out 0x20, al
	pop eax
    iret