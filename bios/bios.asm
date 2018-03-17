main:
    mov r0, &0xfff0
    lsp r0
    mov r0, &msg
    call &debug_print
    hlt

msg db "hello world, written with my sed assembler!", 0x0a, 0x00

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
