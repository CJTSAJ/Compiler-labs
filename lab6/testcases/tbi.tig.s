.section .rodata
.L13:
.int 1
.string "\n"
.section .rodata
.L10:
.int 1
.string "\n"
.section .rodata
.L4:
.int 1
.string "y"
.section .rodata
.L3:
.int 1
.string "x"
.text
.globl tigermain
.type tigermain, @function
tigermain:
subq $8, %rsp
.L15:
movq $8, %r11
leaq 8(%rsp), %r10
movq %r11, -8(%r10)
leaq 8(%rsp), %rdi
call printb
jmp .L14
.L14:
addq $8, %rsp
ret

.text
.globl printb
.type printb, @function
printb:
subq $8, %rsp
.L17:
leaq 8(%rsp), %r11
movq %rdi, -8(%r11)
movq $0, %r11
movq %r11, %r9
leaq 8(%rsp), %r11
movq -8(%r11), %r11
movq -8(%r11), %r11
movq $1, %r10
subq %r10, %r11
cmpq %r11, %r9
jle .L11
.L1:
leaq .L13, %rdi
call print
jmp .L16
.L11:
movq $0, %rbp
leaq 8(%rsp), %r11
movq -8(%r11), %r11
movq -8(%r11), %r11
movq $1, %r10
subq %r10, %r11
cmpq %r11, %rbp
jle .L8
.L2:
leaq .L10, %rdi
call print
movq $1, %r11
addq %r11, %r9
leaq 8(%rsp), %r11
movq -8(%r11), %r11
movq -8(%r11), %r11
movq $1, %r10
subq %r10, %r11
cmpq %r11, %r9
jle .L11
.L18:
.L8:
cmpq %rbp, %r9
jg .L5
.L6:
leaq .L4, %rdi
.L7:
call print
movq $1, %r11
addq %r11, %rbp
leaq 8(%rsp), %r11
movq -8(%r11), %r11
movq -8(%r11), %r11
movq $1, %r10
subq %r10, %r11
cmpq %r11, %rbp
jle .L8
.L19:
.L5:
leaq .L3, %rdi
jmp .L7
.L16:
addq $8, %rsp
ret

