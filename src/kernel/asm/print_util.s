[bits 32]

print_string: ;Will only work in 16-bit mode
	pusha
	mov ah, 0x0e
print_loop:
	mov al, [bx]
	cmp al, 0
	je print_string_done
	int 0x10
	add bx, 1
	jmp print_loop
print_string_done:
	popa
	ret
	
print_char:
	pusha
	mov ah, 0x0e
	int 0x10
	popa
	ret
	
print_hex: ;Will only work in 16 bit mode
	pusha

	mov cx,4
char_loop:
	dec cx

	mov ax,dx
	shr dx,4
	and ax,0xf

	mov bx, HEX_OUT
	add bx, cx

	cmp ax,0xa
	jl set_letter
	add byte [bx],7
	jl set_letter

set_letter:
	add byte [bx],al

	cmp cx,0
	je print_hex_done
	jmp char_loop

print_hex_done:
	mov bx, HEX_OUT
	call print_string
	sub bx,3
	mov cx,4
	call reset_hex_loop
	popa
	ret

reset_hex_loop:
	dec cx
	mov bx, HEX_OUT
	add bx,cx
	mov byte [bx],'0'
	cmp cx,0
	jne reset_hex_loop
	ret

HEX_OUT: db '0000',0