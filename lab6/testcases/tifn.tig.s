.section .rodata
.L7:
.int 1
.string "\n"
.section .rodata
.L3:
.int 20
.string "hey! Bigger than 3!\n"
.text
.globl tigermain
.type tigermain, @function
tigermain:
subq $8, %rsp
.L9:
movq $5, %rax
leaq 8(%rsp), %r11
movq %rax, -8(%r11)
leaq 8(%rsp), %rax
movq -8(%rax), %rdi
call printi
leaq .L7, %rdi
call print
leaq 8(%rsp), %rdi
movq $2, %rsi
call g
leaq 8(%rsp), %rax
movq -8(%rax), %rdi
call printi
jmp .L8
.L8:
addq $8, %rsp
ret

.text
.globl g
.type g, @function
g:
subq $8, %rsp
.L11:
leaq 8(%rsp), %rax
movq %rdi, -8(%rax)
movq $1, %rax
movq $3, %r11
cmpq %r11, %rsi
jg .L1
.L2:
movq $0, %rax
.L1:
movq $0, %r11
cmpq %r11, %rax
jne .L4
.L5:
movq $4, %rax
leaq 8(%rsp), %r11
movq -8(%r11), %r11
movq %rax, -8(%r11)
.L6:
movq $0, %rax
jmp .L10
.L4:
leaq .L3, %rdi
call print
jmp .L6
.L10:
addq $8, %rsp
ret
