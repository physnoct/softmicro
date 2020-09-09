#include "softmicro.cpu"

        nop
        di
        ei
        ret
        pushf
        popf
        call 1000
        jmp 100
        ver
        sern
        halt
start:
        ld.w    r2,#0xBEEF
        ld.d    r0,#0xDEADBEEF
        ld.q    r0,#0xFEEDBABEDEADBEEF
        ld.o    r0,#0x15BADDADFEEDBABEDEADBEEF

#addr 0x200
label1:
        br      label1
        br.w    start
        inc     r0
        inc     r1
        inc     r14
        inc     r15
        cpl     r0
        cpl     r2
        cpl.w   r0
        cpl.w   r2
        vect    3
        wpnz    4,(0x54)
        tog     c
        ld      r1,r2
label2:
        djnz    r1,label2
        rol     r3,2
        rol.q   r3,2
        bitc    r4.1

        brcs    label2

        ld      r1,*r3
        ld      r1,*r3++
        ld      r1,--*r3
        ld      r1,r3[45]
        ld      *r3,r2
        ld      *r3++,r2
        ld      --*r3,r2
        ld      r3[45],r2

        add     r1,*r4
        halt
