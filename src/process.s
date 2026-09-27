.section .text
.global switch_task_context
switch_task_context:
    movl 4(%esp), %eax
    movl 8(%esp), %edx
    testl %edx, %edx
    jnz 1f
    movl %eax, %edx
1:
    movl %edx, %esp

    popl %eax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    popa
    addl $8, %esp
    iret

.section .note.GNU-stack,"",@progbits
.balign 1
