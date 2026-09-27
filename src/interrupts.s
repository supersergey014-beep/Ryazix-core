.section .text

.extern isr_handler
.extern irq_handler
.extern syscall_handler
.extern switch_task_context

.macro ISR_NO_ERROR number
.global isr\number
isr\number:
    pushl $0
    pushl $\number
    jmp isr_common_stub
.endm

.macro ISR_ERROR number
.global isr\number
isr\number:
    pushl $\number
    jmp isr_common_stub
.endm

ISR_NO_ERROR 0
ISR_NO_ERROR 1
ISR_NO_ERROR 2
ISR_NO_ERROR 3
ISR_NO_ERROR 4
ISR_NO_ERROR 5
ISR_NO_ERROR 6
ISR_NO_ERROR 7
ISR_ERROR 8
ISR_NO_ERROR 9
ISR_ERROR 10
ISR_ERROR 11
ISR_ERROR 12
ISR_ERROR 13
ISR_ERROR 14
ISR_NO_ERROR 15
ISR_NO_ERROR 16
ISR_ERROR 17
ISR_NO_ERROR 18
ISR_NO_ERROR 19
ISR_NO_ERROR 20
ISR_ERROR 21
ISR_NO_ERROR 22
ISR_NO_ERROR 23
ISR_NO_ERROR 24
ISR_NO_ERROR 25
ISR_NO_ERROR 26
ISR_NO_ERROR 27
ISR_NO_ERROR 28
ISR_ERROR 29
ISR_ERROR 30
ISR_NO_ERROR 31

.macro IRQ_STUB number
.global irq\number
irq\number:
    pushl $0
    pushl $(32 + \number)
    jmp isr_common_stub
.endm

IRQ_STUB 0
IRQ_STUB 1
IRQ_STUB 2
IRQ_STUB 3
IRQ_STUB 4
IRQ_STUB 5
IRQ_STUB 6
IRQ_STUB 7
IRQ_STUB 8
IRQ_STUB 9
IRQ_STUB 10
IRQ_STUB 11
IRQ_STUB 12
IRQ_STUB 13
IRQ_STUB 14
IRQ_STUB 15

.global syscall_stub
syscall_stub:
    pushl $0
    pushl $128
    jmp isr_common_stub

.global isr_default
isr_default:
    pushl $0
    pushl $255
    jmp isr_common_stub

isr_common_stub:
    pusha
    xorl %eax, %eax
    movw %ds, %ax
    pushl %eax

    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    movl %esp, %ebx
    andl $-16, %esp
    subl $64, %esp
    movl %esp, %edi
    movl %ebx, %esi
    movl $14, %ecx
    cld
    rep movsl

    cmpl $32, 36(%ebx)
    jb .Lisr_exception
    cmpl $48, 36(%ebx)
    jb .Lisr_irq
    cmpl $128, 36(%ebx)
    je .Lisr_syscall

.Lisr_exception:
    call isr_handler

.Lisr_restore_exception:
    movl %ebx, %esp
    popl %eax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    popa
    addl $8, %esp
    iret

.Lisr_irq:
    subl $12, %esp
    pushl %ebx
    call irq_handler
    addl $16, %esp
    jmp .Lisr_switch

.Lisr_syscall:
    subl $12, %esp
    pushl %ebx
    call syscall_handler
    addl $16, %esp

.Lisr_switch:
    subl $8, %esp
    pushl %eax
    pushl %ebx
    call switch_task_context
    ud2

.section .note.GNU-stack,"",@progbits
.balign 1
