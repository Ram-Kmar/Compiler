.global _main
.align 2
.data
msg: .asciz "Val: %d\n"
.text
_main:
    stp x29, x30, [sp, #-16]!
    mov x29, sp
    
    adrp x0, msg@PAGE
    add x0, x0, msg@PAGEOFF
    mov x1, #42
    bl _printf
    
    mov x0, #0
    ldp x29, x30, [sp], #16
    ret