#include "softmicro.cpu"

;STRUCTURED MACROS CAN BE FREELY NESTED AND MIXED AS YOU WISH.

;Now we can put comments in the same line as the macros, but they will not be
;printed in the output file.

mem_ptr = 0x7fff

	;.org     0

        ;-------------------------
		;DO 1   ;Comment _do
		_do

			ld  	r2,#2
			;DO 2.1
			_do

				;TEST OF TWO FOLLOWING DO LOOPS INSIDE
				;A BIGGER DO LOOP.
                ;DO 3.1
				_do
				_do_while_cmp (r2,!=,#0) ;comment M_DO_WHILE
				ld		r3,#3

				;DO 3.2
				_do

					;BREAK
					_do_break

					;REDO
					_redo

				;WHILE (r3,!=,0)
				_do_while_cmp (r3,!=,#0)

				;OTHER
				;BREAK
				_do_break

			;WHILE (r2,!=,0)
			_do_while_cmp (r2,!=,#0)

			;TEST OF TWO FOLLOWING DO LOOPS INSIDE
			;A BIGGER DO LOOP.
			;DO 2.2
			_do
			_do_while_cmp (r2,!=,#0)

			;TEST OF THREE FOLLOWING DO LOOPS INSIDE
			;A BIGGER DO LOOP.
			;DO 2.3
			_do
			_do_while_cmp (r2,!=,#0)

			;OTHER
			;BREAK
			_do_break

		;WHILE (r1,!=,0)
		_do_while_cmp (r1,!=,#0)

        ;----------------------------
        ld      r2,#4
		;DO
		_do

		;WHILE (r2,!=,0)
		_do_while_cmp (r2,!=,#0)
        ;----------------------------
        ;_if_bit       LEVEL 1
        _if_bit(7,r0)

            ;_if_not_bit  LEVEL 2
            _if_not_bit(7,r2)

			;_else		LEVEL 2
            _else

			;_endif	LEVEL 2
			_endif

			;TEST OF TWO FOLLOWING IF INSIDE ANOTHER
			_if_bit(6,r0)
			_endif

		;_else		LEVEL 1
		_else

            ;_if_bit   LEVEL 2
            _if_bit(3,r1)

			;_else		LEVEL 2
			_else

			;_endif	LEVEL 2
			_endif

		;_endif	LEVEL 1
		_endif

.equ VALUE_MASK, 0x0F

        ;----------------------------
		;test switch, no mask
		_sw_case(r1)

		_case (r1,#1)
            ;CODE CASE 1
			_sw_break

		_sw_default
			;CODE
		_sw_end
        ;----------------------------
		_sw_case(r1,#VALUE_MASK)

		_case (r1,#1)
            ;CODE CASE 1
			_sw_break

		_case_or2(r1,#2,#3)
            ;CODE CASE 2 OR 3

                ;NO MASK
                _sw_case(r2,)

                _case(r2,#0x0A)
                ;CODE CASE A
                _sw_break

                _case(r2,#0x0B)
                ;CODE CASE B
                _sw_break

                _sw_default
                ;CODE DEFAULT
                _sw_end

            _sw_break

		_case_or3(r1,#4,#5,#6)
            ;CODE CASE 4,5 OR 6

			;test two following SWITCH inside another
			_sw_case(r0,)
                _case (r0,#21)
				_sw_break
                _sw_default
			_sw_end

			;test two following SWITCH inside another
			_sw_case(r0,)
                _case (r0,#31)
				_sw_break
                _sw_default
			_sw_end

			_sw_break

		_sw_default
			;CODE
		_sw_end

        ;----------------------------
		;IF CMP TESTS

        ;c < b
        _if_cmp(r2,<,r1)
        _endif

        ;c <= b
        _if_cmp(r2,<=,r1)
        _endif

        ;c > b
        _if_cmp(r2,>,r1)

			ld      r2,#0x12
			_do

				INC		r2

			_do_while_cmp(r2,!=,#15)

		_else

			_sw_case(r2,#0x7F)

				_case(r2,#12)
					ld		r0,#0x20
					add		r2,r0
				_sw_break

				_case_or2(r2,#13,#14)
					ld		r2,#0xFF
				_sw_break

				_sw_default
					ld		r2,#0x45

			_sw_end

	    _endif

        ;c >= b
        _if_cmp(r2,>=,r1)
        _endif

        ;c < A
        _if_cmp(r2,<,r0)
        _endif

        ;c <= A
        _if_cmp(r2,<=,r0)
        _endif

        ;c > A
        _if_cmp(r2,>,r0)
        _endif

        ;c >= A
        _if_cmp(r2,>=,r0)
        _endif

        ;A < b
        _if_cmp(r0,<,r1)
        _endif

        ;A <= b
        _if_cmp(r0,<=,r1)
        _endif

        ;A > b
        _if_cmp(r0,>,r1)
        _endif

        ;A >= b
        _if_cmp(r0,>=,r1)
        _endif

        ;c = b
        _if_cmp(r2,=,r1)
        _endif

        ;c = b
        _if_cmp(r2,==,r1)
        _endif

        ;c != b
        _if_cmp(r2,!=,r1)
        _endif

        ;c != b
        _if_cmp(r2,<>,r1)
        _endif

        ;----------------------------
		;WHILE/WH_END TESTS

        ;A = 3E
        _while_equ(r0,#0x3e)
			nop
			nop
        _wh_end

		;A & Mask = 3E
        _while_equm(r0,#VALUE_MASK,#0x3e)
        _wh_end

        ;c >= b
        _while_cmp(r2,>=,r1)
        _wh_end

        ;c < A
        _while_cmp(r2,<,r0)
        _wh_end

        ;c <= A
        _while_cmp(r2,<=,r0)
        _wh_end

        ;c > A
        _while_cmp(r2,>,r0)
        _wh_end

        ;c >= A
        _while_cmp(r2,>=,r0)
        _wh_end

        ;A < b
        _while_cmp(r0,<,r1)
        _wh_end

        ;A <= b
        _while_cmp(r0,<=,r1)
        _wh_end

        ;A > b
        _while_cmp(r0,>,r1)
        _wh_end

        ;A >= b
        _while_cmp(r0,>=,r1)
        _wh_end

        ;c = b
        _while_cmp(r2,=,r1)
        _wh_end

        ;c = b
        _while_cmp(r2,==,r1)
        _wh_end

        ;c != b
        _while_cmp(r2,!=,r1)
        _wh_end

        ;c != b
        _while_cmp(r2,<>,r1)

			;TEST OF TWO FOLLOWING WHILE INSIDE ANOTHER
    	    _while_cmp(r2,<>,r1)
        	_wh_end

	        _while_cmp(r2,<>,r1)
    	    _wh_end

        _wh_end

        ;----------------------------
		;DO/DO_WHILE TESTS

        ;c >= b
        _do
        _do_while_cmp(r2,>=,r1)

        ;c < A
        _do
        _do_while_cmp(r2,<,r0)

        ;c <= A
        _do
        _do_while_cmp(r2,<=,r0)

        ;c > A
        _do
        _do_while_cmp(r2,>,r0)

        ;c >= A
        _do
        _do_while_cmp(r2,>=,r0)

        ;A < b
        _do
        _do_while_cmp(r0,<,r1)

        ;A <= b
        _do
        _do_while_cmp(r0,<=,r1)

        ;A > b
        _do
        _do_while_cmp(r0,>,r1)

        ;A >= b
        _do
        _do_while_cmp(r0,>=,r1)

        ;c = b
        _do
        _do_while_cmp(r2,=,r1)

        ;c = b
        _do
        _do_while_cmp(r2,==,r1)

        ;c != b
        _do
        _do_while_cmp(r2,!=,r1)

        ;c != b
        _do
        _do_while_cmp(r2,<>,r1)

        ;----------------------------
		;DJNZ TEST
		_do
		_do
		_do
		_do
		_do
		_do
		_do
		_do
		_do
		_do
		_do_while_dnz(r0)
		_do_while_dnz(r1)
		_do_while_dnz(r2)
		_do_while_dnz(r3)
		_do_while_dnz(r4)
		_do_while_dnz(r5)
		_do_while_dnz(r6)
		_do_while_dnz(r7)
		_do_while_dnz(r8)
		_do_while_dnz(r9)

null_tests:
        _if_equ(r0,#NULL)
        _if_equ(r0,#0)
        _if_equ(r0,#0x00)
        _endif
        _endif
        _endif

tests:
        ;if_range
        _if_range(r0,#0x30,#0x39+1)
            ;is an ASCII number
        _endif

        ;r1,#3
        _if_equ(r1,#3)
        _endif

        ;r1,#0
        _if_equ(r1,#0)
        _endif

        ;r1,#3,#3
        _if_equm(r1,#3,#3)
        _endif

        ;r1,#3,#0
        _if_equm(r1,#3,#0)
        _endif

        ;r0,#3
        _if_equ(r0,#3)
        _endif

        ;r0,#0
        _if_equ(r0,#0)
        _endif

        ;r0,#3,#3
        _if_equm(r0,#3,#3)
        _endif

        ;r0,#3,#0
        _if_equm(r0,#3,#0)
        _endif

        ;r1,r2,r3
        _if_equm(r1,r2,r3)
        _endif

;test_do_while_range
        _do
;loop:
            ld		r0,#'>

		;If command is not valid, discard and wait for another one
		_do_while_range(r0,r2,r1)

		;cp		c
		;jr		C,loop
		;cp		b
		;jr		NC,loop


