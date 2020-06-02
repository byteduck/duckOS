[bits 32]
disk_load: ;Loads DH sectors into ES:BX from drive DL starting at cl
              ;Only works in 16-bit mode
push dx
mov ah,0x02 ;Read
mov al, dh  ;Read DH sectors
mov ch,0x00 ;Cylinder 0
mov dh,0x00 ;Head 0
disk_load_go:
int 0x13
jc disk_error ;Something went wrong
pop dx
cmp dh,al ;Compare sectors expected vs sectors gotten
jne disk_error
ret

disk_error:
jmp $