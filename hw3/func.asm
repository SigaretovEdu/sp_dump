section .text
	global find_num

; rdi, rsi, rdx, rcx, r8, r9
; rdi -  size
; rsi -  pointer h
; rdx -  pointer w
; rcx -  pointer buffer

find_num:
	push rsi
	push rdx
	mov rbx, rcx
	mov rcx, rdi
	mov rdi, 0
	dec rcx
	mov rax, 0
	mov eax, dword [rbx]
	mov rsi, 1
.L1:
	cmp eax, dword [rbx + rsi * 4]
	jl .L2
	mov eax, dword [rbx + rsi * 4]
	mov rdi, rsi
.L2:
	inc rsi
	loop .L1

	mov rax, rdi
	movsx rcx, dword [rdx]
	mov rdx, 0
	div ecx
	mov rcx, rdx

	pop rdx
	pop rsi

	mov dword [rsi], eax
	mov dword [rdx], ecx

	ret
