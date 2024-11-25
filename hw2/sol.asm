global _start
 
section .data
	pr db "Result value = 1",10
	pr_len equ $-pr

	buffer dw 1, 0, 1, 1, 1, 0, 0, 0, 0, 0
 
section .text
_start:
	mov rax, 0
	mov rcx, 10
mainloop:
	movsx rdx, word [buffer + rcx * 2 - 2]
	and rdx, 1
	add rax, rdx
	loop mainloop

	cmp rax, 5
	je true
false:
	mov rax, 0
	jmp final
true:
	mov rax, 1
final:

	mov [pr + 15], rax
	add byte [pr + 15], "0"

	mov rax, 1
	mov rdi, 1
	mov rsi, pr
	mov rdx, pr_len
	syscall
 
	mov rdi, buffer
    mov rax, 60
    syscall
