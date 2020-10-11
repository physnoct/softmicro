#include "softmicro.cpu"

		ld.w	r15,#0xffe0

		push.w  #0x1234
		push.o  #0xf0e1d2c3b4a5968778695a4b3c2d1e0f
		popn    #18
        halt

table2:
;test_call2,test_call3,test_call4

#d8    test_call1 & 0xff, test_call1 >>8
#d8    test_call2 & 0xff, test_call2 >>8
#d8    test_call3 & 0xff, test_call3 >>8
#d8    test_call4 & 0xff, test_call4 >>8

test_rand:
.local:
        rand.q
        br      .local

test_call1:
        ld      r3,#1
		ret
test_call2:
        ld      r3,#2
		ret
test_call3:
        ld      r3,#3
		ret
test_call4:
        ld      r3,#4
		ret

tested:
test_mcpir:
		ld.w    r1,#table2
		ld.w    r2,#0x2000
		ld.w    r3,#0x20
        mcpir   (r2,r1),r3

test_bsr_cond:
		clr     c
		bsrcc   test_call1
		bsrcs   test_call2
		set     c
		bsrcc   test_call1
		bsrcs   test_call2
test_push_pop:
		ld.o    r0,#0xf0e1d2c3b4a5968778695a4b3c2d1e0f
		push.o  r0
		pop.o   r14
test_bsr_table:
        ld      r0,#2
        bsr     (table2)[r0]
test_ind_call:
		ld.w	r1,#test_call1
        call    (r1)
test_ld_table:
        ld.w    r0,#0
        ld.w    r1,(table_test)[r0]
        inc     r0
        ld.w    r1,(table_test)[r0]

        clr.w   r0
        ld.w    r2,#0x1234
        add.w   r2,(pc+table_test)[r0]
        inc     r0
        add.w   r2,(pc+table_test)[r0]
test_vasm:
		vasm
test_push_pc:
        push    pc

		call	putstring_imm
#str	"Hello World! from softmicro.\0"

		halt
;----------------------------------------------------
putstring_imm:
		;r15 = return add
		ex.w	r1,*r15
.loop:
		ld		r0,*r1++
		sfl		r0
		breq	.loop_end

		out		(0xff),r0
		br		.loop
.loop_end:
		ex.w	*r15,r1
		ret
;----------------------------------------------------
table_test:
#d8     0x35, 0xea, 0x46, 0x95
#res    1024

end_table:







