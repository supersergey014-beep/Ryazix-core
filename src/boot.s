.section .multiboot
.align 4
.long 0x1BADB002
.long 0x00000000
.long -(0x1BADB002 + 0x00000000)

.section .bss
.align 16
.global stack_bottom
.global stack_top
stack_bottom:
    .skip 16384
stack_top:

.section .text
.global _start
.extern kernel_main
_start:
    mov $stack_top, %esp
    and $-16, %esp
    sub $8, %esp
    push %ebx
    push %eax
    call kernel_main

halt_loop:
    cli
    hlt
    jmp halt_loop

.section .note.GNU-stack,"",@progbits
