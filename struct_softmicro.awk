#! /usr/bin/gawk -f

# ----- CPU: SOFTMICRO
@include "struct_macros.awk"
@include "struct_tests.awk"
@include "struct_cond.awk"

# HOW TO USE:
#
# gawk -f struct_softmicro.awk myprog.asm >myprog.result
#
# The file myprog.result will contain the resulting assembler source with
# the macros expanded (code and labels generated).

# ALL MACROS BLOCK TYPES CAN BE FREELY MIXED TOGETHER
# SUPPORTS NESTED IF BLOCKS, DO-WHILE BLOCKS,SWITCHES AND WHILE-END BLOCKS
#
# LEVEL NESTING IS NOT CHECKED, IT'S ASSUMED THAT THE AWK'S STACK SHOULD BE ENOUGH
# FOR MOST APPLICATIONS WITHOUT BUSTING

BEGIN {
# INITIALIZE THE STRUCTURED MACROS SYSTEM
# We generate ourselves the local labels to keep track of them for doing the jumps
	_sp_do_ctr = 0		# DO WHILE LEVEL COUNTER		(nested do while loops)
	_sp_wh_ctr = 0		# WHILE LEVEL COUNTER			(nested while loops)
	_sp_if_ctr = 0		# IF ELSE ENDIF LEVEL COUNTER		(nested if then else)
	_sp_sw_ctr = 0		# SWITCH LEVEL COUNTER			(nested switches)
	_sp_case_ctr = 0	# SWITCH CASE COUNTER

	_sp_stack_ptr = 0	# STACK POINTER
	_sp_line_number = 1	# LINE COUNTER
	_sp_label = 1

	# LET AWK DO THE PARSING OF ARGUMENTS FOR US
	FS = "[(,)]"
}

# CODE EXECUTED FOR EACH LINE OF THE FILE
{
	# Is it a comment?
	if ($1 ~ /;/)
	{
		print $0
	}
	else
	{
	    # There should not be any label in the same line as the macro
	    # it's not necessary anyway

	    if ($1 ~ /ANL/) {ANL($2,$3)}
	    else if ($1 ~ /JNE/) {JNE($2,$3,$4)}
	    else if ($1 ~ /JEQ/) {JEQ($2,$3,$4)}

	#=============================================================
	# You can change macro names as you wish !
	#=============================================================
	# Structures do { break(cond) } while (cond);
	    else if ($1 ~ /_do_while_z/) {M_DO_WHILE_Z()}
	    else if ($1 ~ /_do_while_nz/) {M_DO_WHILE_NZ()}
	    else if ($1 ~ /_do_while_cy/) {M_DO_WHILE_CY()}
	    else if ($1 ~ /_do_while_nc/) {M_DO_WHILE_NC()}
	    else if ($1 ~ /_do_while_plus/) {M_DO_WHILE_PLUS()}
	    else if ($1 ~ /_do_while_minus/) {M_DO_WHILE_MINUS()}
	    else if ($1 ~ /_do_while_bit/) {M_DO_WHILE_BIT($2,$3)}
	    else if ($1 ~ /_do_while_not_bit/) {M_DO_WHILE_NOT_BIT($2,$3)}
	    else if ($1 ~ /_do_while_equm/) {M_DO_WHILE_EQU($2,$3,$4)}
	    else if ($1 ~ /_do_while_neqm/) {M_DO_WHILE_NOT_EQU($2,$3,$4)}
	    else if ($1 ~ /_do_while_equ/) {M_DO_WHILE_EQU($2,$3)}
	    else if ($1 ~ /_do_while_neq/) {M_DO_WHILE_NOT_EQU($2,$3)}
	    else if ($1 ~ /_do_while_cmp/) {M_DO_WHILE_CMP($2,$3,$4)}
	    else if ($1 ~ /_do_while_dnz/) {M_DO_DEC_WHILE_NZ($2)}
	    else if ($1 ~ /_do_while_range/) {M_DO_WHILE_RANGE($2,$3,$4)}
	    else if ($1 ~ /_do_while_always/) {M_DO_WHILE_ALWAYS()}
	    else if ($1 ~ /_do_loop/) {M_DO_WHILE_ALWAYS()}
	    else if ($1 ~ /_do_break_nz/) {M_DO_BREAK_NZ()}
	    else if ($1 ~ /_do_break_z/) {M_DO_BREAK_Z()}
	    else if ($1 ~ /_do_break_nc/) {M_DO_BREAK_NC()}
	    else if ($1 ~ /_do_break_cy/) {M_DO_BREAK_CY()}
	    else if ($1 ~ /_do_break_range/) {M_DO_BREAK_RANGE($2,$3,$4)}
	    else if ($1 ~ /_do_break/) {M_DO_BREAK()}
	    else if ($1 ~ /_redo_nz/) {M_REDO_NZ()}
	    else if ($1 ~ /_redo_z/) {M_REDO_Z()}
	    else if ($1 ~ /_redo/) {M_REDO()}
		else if ($1 ~ /_do_end/) {M_DO_END()}
	    else if ($1 ~ /_do/) {M_DO()}

	# Structures while (cond) { break[cond] };
	    else if ($1 ~ /_while_z/) {M_WHILE_Z()}
	    else if ($1 ~ /_while_nz/) {M_WHILE_NZ()}
	    else if ($1 ~ /_while_bit/) {M_WHILE_BIT($2,$3)}
	    else if ($1 ~ /_while_not_bit/) {M_WHILE_NOT_BIT($2,$3)}
	    else if ($1 ~ /_while_equm/) {M_WHILE_EQUM($2,$3,$4)}
	    else if ($1 ~ /_while_equ/) {M_WHILE_EQU($2,$3)}
	    else if ($1 ~ /_while_cmp/) {M_WHILE_CMP($2,$3,$4)}
	    else if ($1 ~ /_while_range/) {M_WHILE_RANGE($2,$3,$4)}
	    else if ($1 ~ /_wh_break/) {M_WH_BREAK()}
	    else if ($1 ~ /_wh_end/) {M_WH_END()}

	# Structures if (cond) {} else {};
	    else if ($1 ~ /_if_plus/) {M_IF_PLUS()}
	    else if ($1 ~ /_if_minus/) {M_IF_MINUS()}

	    else if ($1 ~ /_if_z/) {M_IF_Z()}
	    else if ($1 ~ /_if_nz/) {M_IF_NZ()}
	    else if ($1 ~ /_if_cy/) {M_IF_CY()}
	    else if ($1 ~ /_if_nc/) {M_IF_NC()}
	    else if ($1 ~ /_if_bit/) {M_IF_BIT($2,$3)}
	    else if ($1 ~ /_if_not_bit/) {M_IF_NOT_BIT($2,$3)}
	    else if ($1 ~ /_if_range/) {M_IF_RANGE($2,$3,$4)}

	    else if ($1 ~ /_if_equm_or2/) {M_IF_EQUM_OR2($2,$3,$4,$5)}
	    else if ($1 ~ /_if_equm_or3/) {M_IF_EQUM_OR3($2,$3,$4,$5,$6)}
	    else if ($1 ~ /_if_equm_or4/) {M_IF_EQUM_OR4($2,$3,$4,$5,$6,$7)}
	    else if ($1 ~ /_if_equm/) {M_IF_EQUM($2,$3,$4)}
	    else if ($1 ~ /_if_neqm/) {M_IF_NOT_EQUM($2,$3,$4)}

	    else if ($1 ~ /_if_equ_or2/) {M_IF_EQU_OR2($2,$3,$4)}
	    else if ($1 ~ /_if_equ_or3/) {M_IF_EQU_OR3($2,$3,$4,$5)}
	    else if ($1 ~ /_if_equ_or4/) {M_IF_EQU_OR4($2,$3,$4,$5,$6)}
	    else if ($1 ~ /_if_equ/) {M_IF_EQU($2,$3)}
	    else if ($1 ~ /_if_neq/) {M_IF_NOT_EQU($2,$3)}
	    else if ($1 ~ /_if_cmp/) {M_IF_CMP($2,$3,$4)}
	    else if ($1 ~ /_elseif_bit/) {M_ELSE_IF_BIT($2,$3)}
	    else if ($1 ~ /_elseif_not_bit/) {M_ELSE_IF_NOT_BIT($2,$3)}
	    else if ($1 ~ /_elseif_range/) {M_ELSE_IF_RANGE($2,$3)}
	    else if ($1 ~ /_elseif_equ_or2/) {M_ELSE_IF_EQU_OR2($2,$3)}
	    else if ($1 ~ /_elseif_equ_or3/) {M_ELSE_IF_EQU_OR3($2,$3,$4)}
	    else if ($1 ~ /_elseif_equ_or4/) {M_ELSE_IF_EQU_OR4($2,$3,$4,$5)}
	    else if ($1 ~ /_elseif_equ/) {M_ELSE_IF_EQU($2)}
	    else if ($1 ~ /_nj_else/) {M_NO_JUMP_ELSE()}
	    else if ($1 ~ /_else/) {M_ELSE()}
	    else if ($1 ~ /_endif/) {M_ENDIF()}

	    else if ($1 ~ /_switch/) {M_SWITCH($2,$3)}
	    else if ($1 ~ /_sw_io/) {M_SWITCH_IO($2)}
	    else if ($1 ~ /_sw_case/) {M_SWITCH($2,$3)}
	    else if ($1 ~ /_case_or2/) {M_CASE_OR2($2,$3,$4)}
	    else if ($1 ~ /_case_or3/) {M_CASE_OR3($2,$3,$4,$5)}
	    else if ($1 ~ /_case_or4/) {M_CASE_OR4($2,$3,$4,$5,$6)}
	    else if ($1 ~ /_case_range/) {M_CASE_RANGE($2,$3,$4)}
	    else if ($1 ~ /_case_cmp/) {M_CASE_CMP($2,$3,$4)}
	    else if ($1 ~ /_case_bit/) {M_CASE_BIT($2,$3)}
	    else if ($1 ~ /_case_not_bit/) {M_CASE_NOT_BIT($2,$3)}
	    else if ($1 ~ /_case/) {M_CASE($2,$3)}
	    else if ($1 ~ /_sw_break_z/) {M_SW_BREAK_Z()}
	    else if ($1 ~ /_sw_break_nz/) {M_SW_BREAK_NZ()}
	    else if ($1 ~ /_sw_break/) {M_SW_BREAK()}

		# ALLOW SOME FREEDOM
	    else if ($1 ~ /_default|_sw_default/) {M_SW_DEFAULT()}

	    else if ($1 ~ /_sw_end/) {M_SW_END()}

		else if ($1 ~ /_break/)
		{
			if (_sp_current_block ~ "SWITCH") {M_SW_BREAK()}
			else if (_sp_current_block ~ "WHILE") {M_WH_BREAK()}
			else if (_sp_current_block ~ "DO") {M_DO_BREAK()}
			else if (_sp_current_block ~ "IF") {M_IF_BREAK()}
  		}

		# commas in string don't work
		else if ($1 ~ /_print/) {PRINT_STR($2)}
		else
			print $0

	}
	_sp_line_number++
}

END {
}
