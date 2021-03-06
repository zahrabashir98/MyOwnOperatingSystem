.set IRQ_BASE, 0x20
.section .text

.extern _ZN4myos21hardwarecommunication16InterruptManager15handleInterruptEhj
.global _ZN4myos21hardwarecommunication16InterruptManager22IgnoreInterruptRequestEv


.macro HandleException num
.global _ZN4myos21hardwarecommunication16InterruptManager16HandleException\num\()Ev
_ZN4myos21hardwarecommunication16InterruptManager16HandleInterruptRequest\num\()Ev:
    movb $\num, (interruptNumber)
    jmp int_bottom
.endm



.macro HandleInterruptRequest num
.global _ZN4myos21hardwarecommunication16InterruptManager26HandleInterruptRequest\num\()Ev
_ZN4myos21hardwarecommunication16InterruptManager26HandleInterruptRequest\num\()Ev:
    movb $\num + IRQ_BASE, (interruptNumber)
    pushl $0
    jmp int_bottom
.endm


# for more interrupts just copy this
HandleInterruptRequest 0x00
HandleInterruptRequest 0x01
HandleInterruptRequest 0x0C

# jumps into handle interrupt function
int_bottom:

    pusha
    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs


    push esp
    push (interruptNumber)
    call _ZN4myos21hardwarecommunication16InterruptManager15handleInterruptEhj
    # addl $5, %esp 
    movl %eax, %esp
    

    popl %gs
    popl %fs
    popl %es
    popl %ds
    popa


    add $4, %esp


    _ZN4myos21hardwarecommunication16InterruptManager22IgnoreInterruptRequestEv:
    iret


.data
    interruptNumber : .byte 0