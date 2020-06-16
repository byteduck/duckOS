[bits 32]
[global _start]

; A simple program that prompts you to type something and then repeats it back to you.

section .text

_start:

print_prompt:
    mov eax, 4 ;SYS_WRITE
    mov ebx, 1 ;STDOUT
    mov ecx, prompt
    mov edx, 16
    int 0x80

wait_for_input:
    mov eax, 3 ;SYS_READ
    mov ebx, 0 ;STDIN
    mov ecx, buf
    mov edx, 255
    int 0x80

    cmp eax, 0
    je wait_for_input
    mov [num_read], eax

print_input:
    mov eax, 4 ;SYS_WRITE
    mov ebx, 1 ;STDOUT
    mov ecx, response
    mov edx, 11
    int 0x80

    mov eax, 4 ;SYS_WRITE
    mov ecx, buf
    mov edx, [num_read]
    int 0x80

_exit:
    mov eax, 1 ;SYS_EXIT
    int 0x80

section .data

prompt:
    db `Type something: `
response:
    db `You typed: `
buf:
    resb 255
num_read:
    dd 0