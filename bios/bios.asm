main:
    mov r0, &0xfff0
    lsp r0
  .loop:
    mov r0, &msg
    call &debug_print
    mov r0, &buffer
    call &debug_input
    call &debug_print
    mov r0, &newline
    call &debug_print
    jmp &.loop

msg db "CRISP BIOS SHELL> ", 0x00
buffer times 256 db 0
newline db 0x0a, 0x00

debug_print:
    push r0
    push r1
    push r2
    mov r2, &0x40
  .loop:
    loadb r1, r0
    inc r0
    cmp r1, &0
    ifeq jmp &.out
    outb r2, r1
    jmp &.loop
  .out:
    pop r2
    pop r1
    pop r0
    ret

debug_input:
    push r0
    push r1
    push r2
    mov r2, &0x40
  .loop:
    inb r1, r2
    cmp r1, &0
    ifeq jmp &.loop
    outb r2, r1
    cmp r1, &0x0a
    ifeq jmp &.out
    storeb r0, r1
    inc r0
    jmp &.loop
  .out:
    mov r1, &0
    storeb r0, r1
    pop r2
    pop r1
    pop r0
    ret
