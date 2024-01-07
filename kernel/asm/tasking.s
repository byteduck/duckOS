[extern preempt]
[extern tasking_enabled]
[extern preempt_finish]

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
    pop edx
    pop ecx
    pop ebx
    pop eax
    iret

[global preempt_asm]
preempt_asm:
    pushfd
    push ebx
    push esi
    push edi
    push ebp
    mov eax, [esp+24] ;old_esp
    mov ecx, [esp+28] ;new_esp
    mov edx, [esp+32] ;new_cr3
    mov [eax], esp
    mov cr3, edx
    mov esp, [ecx]
    pop ebp
    pop edi
    pop esi
    pop ebx
    popfd
    ret

[global proc_first_preempt]
proc_first_preempt:
    call preempt_finish
    pop gs
    pop fs
    pop es
    pop ds
    pop esi
    pop edi
    pop ebp
    pop edx
    pop ecx
    pop ebx
	mov eax, 0x20
	out 0x20, al
	out 0xA0, al
	pop eax
	fninit
    iret
