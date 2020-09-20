#include "softmicro.cpu"

		ld.w	r15,#0xffe0
		ld.w	r1,#0xaa55

		vasm

		call	putstring_imm
;		call	test_call
;		halt


#str	"Hello World! from softmicro.\0"
;#str	"Yo\0"

		halt

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

test_call:
		ret










