#----------------------------------------------------------
function DO_WHILE_END()
{
	# PUT A LABEL FOR BREAKING OUT OF THE WHILE LOOP
	LABEL3("DO_BRK_",_sp_do_ctr,_sp_do_lbl[_sp_do_ctr])

	_sp_do_lbl[_sp_do_ctr]++
	_sp_do_ctr--

	# BLOCK BEGIN/END NOT MATCHING
	if (_sp_do_ctr < 0) ERR_MSG("M_DO_WHILE WITHOUT M_DO")
}

function M_DO()
{
	_sp_current_block = "DO"
	_sp_do_ctr++

	# BEGINNING OF THE LOOP, MUST JUMP HERE
	LABEL3("DO_",_sp_do_ctr,_sp_do_lbl[_sp_do_ctr])
}

function M_DO_END()
{
	DO_WHILE_END()
}

function M_DO_WHILE_ALWAYS()
{
	#JUMP TO THE BEGINNING OF THE DO WHILE LOOP
	JMP_ALWAYS("DO_",_sp_do_ctr,_sp_do_lbl[_sp_do_ctr])

	DO_WHILE_END()
}

function M_DO_WHILE_BIT(BIT,REGISTER)
{
	JMP_BIT3(BIT,REGISTER,"DO_",_sp_do_ctr,_sp_do_lbl[_sp_do_ctr])
	DO_WHILE_END()
}

function M_DO_WHILE_NOT_BIT(BIT,REGISTER)
{
	JMP_NOT_BIT3(BIT,REGISTER,"DO_",_sp_do_ctr,_sp_do_lbl[_sp_do_ctr])
	DO_WHILE_END()
}

function M_DO_WHILE_Z()
{
	INST3("breq","DO_",_sp_do_ctr,_sp_do_lbl[_sp_do_ctr])
	DO_WHILE_END()
}

function M_DO_WHILE_NZ()
{
	INST3("brne","DO_",_sp_do_ctr,_sp_do_lbl[_sp_do_ctr])
	DO_WHILE_END()
}

function M_DO_WHILE_CY()
{
	INST3("brcs","DO_",_sp_do_ctr,_sp_do_lbl[_sp_do_ctr])
	DO_WHILE_END()
}

function M_DO_WHILE_NC()
{
	INST3("brcc","DO_",_sp_do_ctr,_sp_do_lbl[_sp_do_ctr])
	DO_WHILE_END()
}

function M_DO_WHILE_PLUS()
{
	INST3("brpl","DO_",_sp_do_ctr,_sp_do_lbl[_sp_do_ctr])
	DO_WHILE_END()
}

function M_DO_WHILE_MINUS()
{
	INST3("brmi","DO_",_sp_do_ctr,_sp_do_lbl[_sp_do_ctr])
	DO_WHILE_END()
}

function M_DO_WHILE_EQU(REGISTER,VALUE)
{
    M_SET_FLAGS(REGISTER,VALUE)
    INST3("breq","DO_",_sp_do_ctr,_sp_do_lbl[_sp_do_ctr])
	DO_WHILE_END()
}

function M_DO_WHILE_NOT_EQU(REGISTER,VALUE)
{
    M_SET_FLAGS(REGISTER,VALUE)
    INST3("brne","DO_",_sp_do_ctr,_sp_do_lbl[_sp_do_ctr])
	DO_WHILE_END()
}

function M_DO_WHILE_EQUM(REGISTER,MASK,VALUE)
{
    M_SETM_FLAGS(REGISTER,MASK,VALUE)
    INST3("breq","DO_",_sp_do_ctr,_sp_do_lbl[_sp_do_ctr])
	DO_WHILE_END()
}

function M_DO_WHILE_NOT_EQUM(REGISTER,MASK,VALUE)
{
    M_SETM_FLAGS(REGISTER,MASK,VALUE)
    INST3("brne","DO_",_sp_do_ctr,_sp_do_lbl[_sp_do_ctr])
	DO_WHILE_END()
}

function M_DO_WHILE_CMP(REGISTER,CONDITION,VALUE)
{
	M_COMPARE_NJMP3(REGISTER,CONDITION,VALUE,"DO_",_sp_do_ctr,_sp_do_lbl[_sp_do_ctr])
	DO_WHILE_END()
}

function M_DO_WDEC_WHILE_NZ(REGISTER)
{
    WDEC_JUMP_NZ(REGISTER,"DO_",_sp_do_ctr,_sp_do_lbl[_sp_do_ctr])
	DO_WHILE_END()
}

function M_DO_DEC_WHILE_NZ(REGISTER)
{
    DEC_JUMP_NZ(REGISTER,"DO_",_sp_do_ctr,_sp_do_lbl[_sp_do_ctr])
	DO_WHILE_END()
}

function M_DO_WHILE_RANGE(REG,VALUE1,VALUE2)
{
	IN_RANGE3(REG,VALUE1,VALUE2,"DO_",_sp_do_ctr,_sp_do_lbl[_sp_do_ctr])
	DO_WHILE_END()
}

function M_DO_BREAK_RANGE(REG,VALUE1,VALUE2)
{
	IN_RANGE3(REG,VALUE1,VALUE2,"DO_BRK_",_sp_do_ctr,_sp_do_lbl[_sp_do_ctr])
}

function M_DO_BREAK_Z()
{
	INST3("breq","DO_BRK_",_sp_do_ctr,_sp_do_lbl[_sp_do_ctr])
}

function M_DO_BREAK_NZ()
{
	INST3("brne","DO_BRK_",_sp_do_ctr,_sp_do_lbl[_sp_do_ctr])
}

function M_DO_BREAK_CY()
{
	INST3("brcs","DO_BRK_",_sp_do_ctr,_sp_do_lbl[_sp_do_ctr])
}

function M_DO_BREAK_NC()
{
	INST3("brcc","DO_BRK_",_sp_do_ctr,_sp_do_lbl[_sp_do_ctr])
}

function M_DO_BREAK()
{
	INST3("br","DO_BRK_",_sp_do_ctr,_sp_do_lbl[_sp_do_ctr])
}

function M_REDO()
{
	INST3("br","DO_",_sp_do_ctr,_sp_do_lbl[_sp_do_ctr])
}

function M_REDO_Z()
{
	INST3("breq","DO_",_sp_do_ctr,_sp_do_lbl[_sp_do_ctr])
}

function M_REDO_NZ()
{
	INST3("brne","DO_",_sp_do_ctr,_sp_do_lbl[_sp_do_ctr])
}

#----------------------------------------------------------
function WHILE_BEGIN()
{
	_sp_current_block = "WHILE"

	_sp_wh_ctr++
	LABEL3("WHILE_",_sp_wh_ctr,_sp_wh_lbl[_sp_wh_ctr])
}

function M_WHILE_Z()
{
	WHILE_BEGIN()
	INST3("brne\t","WH_END_",_sp_wh_ctr,_sp_wh_lbl[_sp_wh_ctr])
}

function M_WHILE_NZ()
{
	WHILE_BEGIN()
	INST3("breq\t","WH_END_",_sp_wh_ctr,_sp_wh_lbl[_sp_wh_ctr])
}

function M_WHILE_BIT(BIT,REGISTER)
{
	WHILE_BEGIN()
	JMP_NOT_BIT3(BIT,REGISTER,"WH_END_",_sp_wh_ctr,_sp_wh_lbl[_sp_wh_ctr])
}

function M_WHILE_NOT_BIT(BIT,REGISTER)
{
	WHILE_BEGIN()
	JMP_BIT3(BIT,REGISTER,"WH_END_",_sp_wh_ctr,_sp_wh_lbl[_sp_wh_ctr])
}

function M_WHILE_EQUM(REGISTER,MASK,VALUE)
{
	WHILE_BEGIN()
    M_SETM_FLAGS(REGISTER,MASK,VALUE)
    INST3("brne","WH_END_",_sp_wh_ctr,_sp_wh_lbl[_sp_wh_ctr])
}

function M_WHILE_EQU(REGISTER,VALUE)
{
	WHILE_BEGIN()
    M_SET_FLAGS(REGISTER,VALUE)
    INST3("brne","WH_END_",_sp_wh_ctr,_sp_wh_lbl[_sp_wh_ctr])
}

function M_WHILE_NOT_EQU(REGISTER,VALUE)
{
	WHILE_BEGIN()
    M_SET_FLAGS(REGISTER,VALUE)
    INST3("breq\t","WH_END_",_sp_wh_ctr,_sp_wh_lbl[_sp_wh_ctr])
}

function M_WHILE_NOT_EQUM(REGISTER,MASK,VALUE)
{
	WHILE_BEGIN()
    M_SETM_FLAGS(REGISTER,MASK,VALUE)
    INST3("breq\t","WH_END_",_sp_wh_ctr,_sp_wh_lbl[_sp_wh_ctr])
}

function M_WHILE_CMP(REGISTER,CONDITION,VALUE)
{
	WHILE_BEGIN()
	M_COMPARE_JMP3(REGISTER,CONDITION,VALUE,"WH_END_",_sp_wh_ctr,_sp_wh_lbl[_sp_wh_ctr])
}

function M_WHILE_RANGE(REG,VALUE1,VALUE2)
{
	WHILE_BEGIN()
	IN_RANGE3(REG,VALUE1,VALUE2,"WH_END_",_sp_wh_ctr,_sp_wh_lbl[_sp_wh_ctr])
}

function M_WH_BREAK()
{
	INST3("br","WH_END_",_sp_wh_ctr,_sp_wh_lbl[_sp_wh_ctr])
}

function M_WH_END()
{
	INST3("br","WHILE_",_sp_wh_ctr,_sp_wh_lbl[_sp_wh_ctr])
	LABEL3("WH_END_",_sp_wh_ctr,_sp_wh_lbl[_sp_wh_ctr])

	_sp_wh_lbl[_sp_wh_ctr]++
	_sp_wh_ctr--

	# BLOCK BEGIN/END NOT MATCHING
	if (_sp_wh_ctr < 0) ERR_MSG("M_WH_END WITHOUT M_WHILE_XXX")
}

#----------------------------------------------------------
function IF_BEGIN()
{
	_sp_current_block = "IF"

	PUSH(_sp_is_else)
	_sp_is_else = 0

	PUSH(_sp_if_nb)
	_sp_if_nb = 1

	_sp_if_ctr++
}

function M_IF_PLUS()
{
	IF_BEGIN()

	# JUMP NOT CONDITION
	INST3("brmi\t","ELSE_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr],_sp_if_nb)
}

function M_IF_MINUS()
{
	IF_BEGIN()

	# JUMP NOT CONDITION
	INST3("brpl\t","ELSE_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr],_sp_if_nb)
}

function M_IF_Z()
{
	IF_BEGIN()

	# JUMP NOT CONDITION
	INST3("brne\t","ELSE_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr],_sp_if_nb)
}

function M_IF_NZ()
{
	IF_BEGIN()

	# JUMP NOT CONDITION
	INST3("breq\t","ELSE_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr],_sp_if_nb)
}

function M_IF_CY()
{
	IF_BEGIN()

	# JUMP NOT CONDITION
	INST3("brcc\t","ELSE_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr],_sp_if_nb)
}

function M_IF_NC()
{
	IF_BEGIN()

	# JUMP NOT CONDITION
	INST3("brcs\t","ELSE_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr],_sp_if_nb)
}

function M_IF_BIT(BIT,REGISTER)
{
	IF_BEGIN()

	# JUMP NOT CONDITION
	JMP_NOT_BIT3(BIT,REGISTER,"ELSE_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr],_sp_if_nb)
}

function M_IF_NOT_BIT(BIT,REGISTER)
{
	IF_BEGIN()

	# JUMP if CONDITION
	JMP_BIT3(BIT,REGISTER,"ELSE_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr],_sp_if_nb)
}

function M_IF_NOT_EQU(REGISTER,VALUE)
{
	IF_BEGIN()
    M_SET_FLAGS(REGISTER,VALUE)
    INST3("breq\t","ELSE_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr],_sp_if_nb)
}

function M_IF_EQU(REGISTER,VALUE)
{
	IF_BEGIN()
    M_SET_FLAGS(REGISTER,VALUE)
    INST3("brne","ELSE_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr],_sp_if_nb)
}

function M_IF_EQU_OR2(REGISTER,VALUE1,VALUE2)
{
	IF_BEGIN()
	JNE_OR2(REGISTER,VALUE1,VALUE2,"ELSE_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr],_sp_if_nb)
}

function M_IF_EQU_OR3(REGISTER,VALUE1,VALUE2,VALUE3)
{
	IF_BEGIN()
	JNE_OR3(REGISTER,VALUE1,VALUE2,VALUE3,"ELSE_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr],_sp_if_nb)
}

function M_IF_EQU_OR4(REGISTER,VALUE1,VALUE2,VALUE3,VALUE4)
{
	IF_BEGIN()
	JNE_OR4(REGISTER,VALUE1,VALUE2,VALUE3,VALUE4,"ELSE_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr],_sp_if_nb)
}

function M_IF_NOT_EQUM(REGISTER,MASK,VALUE)
{
	IF_BEGIN()
    M_SETM_FLAGS(REGISTER,MASK,VALUE)
    INST3("breq\t","ELSE_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr],_sp_if_nb)
}

function M_IF_EQUM(REGISTER,MASK,VALUE)
{
	IF_BEGIN()
    M_SETM_FLAGS(REGISTER,MASK,VALUE)
    INST3("brne","ELSE_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr],_sp_if_nb)
}

function M_IF_EQUM_OR2(REGISTER,MASK,VALUE1,VALUE2)
{
	IF_BEGIN()
	ANL(REGISTER,MASK)
	JNE_OR2(REGISTER,VALUE1,VALUE2,"ELSE_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr],_sp_if_nb)
}

function M_IF_EQUM_OR3(REGISTER,MASK,VALUE1,VALUE2,VALUE3)
{
	IF_BEGIN()
	ANL(REGISTER,MASK)
	JNE_OR3(REGISTER,VALUE1,VALUE2,VALUE3,"ELSE_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr],_sp_if_nb)
}

function M_IF_EQUM_OR4(REGISTER,MASK,VALUE1,VALUE2,VALUE3,VALUE4)
{
	IF_BEGIN()
	ANL(REGISTER,MASK)
	JNE_OR4(REGISTER,VALUE1,VALUE2,VALUE3,VALUE4,"ELSE_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr],_sp_if_nb)
}

function M_IF_RANGE(REGISTER,VALUE1,VALUE2)
{
	IF_BEGIN()
	IN_RANGE3(REGISTER,VALUE1,VALUE2,"ELSE_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr],_sp_if_nb)
}

function M_IF_CMP(REGISTER,CONDITION,VALUE)
{
	IF_BEGIN()
	M_COMPARE_JMP3(REGISTER,CONDITION,VALUE,"ELSE_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr],_sp_if_nb)
}

function M_ELSE()
{
	INST3("br","ENDIF_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr])
	LABEL3("ELSE_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr],_sp_if_nb++)
	_sp_is_else = 1
}

function M_NO_JUMP_ELSE()
{
	LABEL3("ELSE_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr],_sp_if_nb++)
	_sp_is_else = 1
}

function M_ELSE_IF_BIT(BIT,REGISTER)
{
	M_ELSE()
	JMP_NOT_BIT3(BIT,REGISTER,"ELSE_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr],_sp_if_nb)
}

function M_ELSE_IF_NOT_BIT(BIT,REGISTER)
{
	M_ELSE()
	JMP_BIT3(BIT,REGISTER,"ELSE_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr],_sp_if_nb)
}

function M_ELSE_IF_EQU(REGISTER,VALUE)
{
	M_ELSE()
	JNE3(REGISTER,VALUE,"ELSE_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr],_sp_if_nb)
}

function M_ELSE_IF_EQU_OR2(REGISTER,VALUE1,VALUE2)
{
	M_ELSE()
	JNE_OR2(REGISTER,VALUE1,VALUE2,"ELSE_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr],_sp_if_nb)
}

function M_ELSE_IF_EQU_OR3(REGISTER,VALUE1,VALUE2,VALUE3)
{
	M_ELSE()
	JNE_OR3(REGISTER,VALUE1,VALUE2,VALUE3,"ELSE_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr],_sp_if_nb)
}

function M_ELSE_IF_EQU_OR4(REGISTER,VALUE1,VALUE2,VALUE3,VALUE4)
{
	M_ELSE()
	JNE_OR4(REGISTER,VALUE1,VALUE2,VALUE3,VALUE4,"ELSE_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr],_sp_if_nb)
}

function M_ELSE_IF_RANGE(REGISTER,VALUE1,VALUE2)
{
	M_ELSE()
	IN_RANGE3(REGISTER,VALUE1,VALUE2,"ELSE_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr],_sp_if_nb)
}

function M_ENDIF()
{
	# THE "IF" PLACED A JUMP TO A LABEL "ELSE_..." THAT WHOULD BE
	# DEFINED IN THE "ELSE"	# MACRO. IF IT WAS NOT DEFINED,
	# (NO "ELSE" SECTION) DEFINE IT HERE.
	if (_sp_is_else == 0)
		LABEL3("ELSE_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr],_sp_if_nb)

	LABEL3("ENDIF_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr])

	# RESTORE FLAGS AND COUNTERS FOR PREVIOUS LEVEL
	_sp_if_nb = POP()
	_sp_is_else = POP()

	# BLOCK BEGIN/END NOT MATCHING
	_sp_if_lbl[_sp_if_ctr]++
	_sp_if_ctr--
	if (_sp_if_ctr < 0) ERR_MSG("M_ENDIF WITHOUT M_IF")
}

function M_IF_BREAK()
{
	INST3("br","ENDIF_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr])
}

#----------------------------------------------------------
function CASE_LABEL()
{
	# GENERATE LABEL FOR CURRENT CASE
	LABEL3("SW_",_sp_sw_ctr,_sp_sw_lbl[_sp_sw_ctr],_sp_case_ctr)

	#INCREMENT CASE COUNTER
	_sp_case_ctr++
}

function M_SWITCH_BEGIN()
{
	_sp_current_block = "SWITCH"
	_sp_sw_ctr++

	# IN CASE OF NESTED SWITCH
	PUSH(_sp_case_ctr)
	_sp_case_ctr = 1

	LABEL3("SW_",_sp_sw_ctr,_sp_sw_lbl[_sp_sw_ctr])
}

function M_SWITCH(REGISTER,MASK)
{
	M_SWITCH_BEGIN()

	ANL(REGISTER,MASK)
}

function M_SWITCH_IO(PORT)
{
	M_SWITCH_BEGIN()

	printf("\tin\tr0,(%s)\n",PORT)
}

function M_CASE(REGISTER,VALUE)
{
	CASE_LABEL()
	JNE3(REGISTER,VALUE,"SW_",_sp_sw_ctr,_sp_sw_lbl[_sp_sw_ctr],_sp_case_ctr)
}

function M_CASE_RANGE(REGISTER,VALUE1,VALUE2)
{
	CASE_LABEL()
	IN_RANGE3(REGISTER,VALUE1,VALUE2,"SW_",_sp_sw_ctr,_sp_sw_lbl[_sp_sw_ctr],_sp_case_ctr)
}

function M_CASE_CMP(REGISTER,CONDITION,VALUE)
{
	CASE_LABEL()
	M_COMPARE_JMP3(REGISTER,CONDITION,VALUE,"ELSE_",_sp_if_ctr,_sp_if_lbl[_sp_if_ctr],_sp_if_nb)
}

function M_CASE_OR2(REGISTER,VALUE1,VALUE2)
{
	CASE_LABEL()
	JNE_OR2(REGISTER,VALUE1,VALUE2,"SW_",_sp_sw_ctr,_sp_sw_lbl[_sp_sw_ctr],_sp_case_ctr)
}

function M_CASE_OR3(REGISTER,VALUE1,VALUE2,VALUE3)
{
	CASE_LABEL()
	JNE_OR3(REGISTER,VALUE1,VALUE2,VALUE3,"SW_",_sp_sw_ctr,_sp_sw_lbl[_sp_sw_ctr],_sp_case_ctr)
}

function M_CASE_OR4(REGISTER,VALUE1,VALUE2,VALUE3,VALUE4)
{
	CASE_LABEL()
	JNE_OR4(REGISTER,VALUE1,VALUE2,VALUE3,VALUE4,"SW_",_sp_sw_ctr,_sp_sw_lbl[_sp_sw_ctr],_sp_case_ctr)
}

function M_CASE_BIT(BIT,REG)
{
	CASE_LABEL()
	JMP_NOT_BIT3(BIT,REG,"SW_",_sp_sw_ctr,_sp_sw_lbl[_sp_sw_ctr],_sp_case_ctr)
}

function M_CASE_NOT_BIT(BIT,REG)
{
	CASE_LABEL()
	JMP_BIT3(BIT,REG,"SW_",_sp_sw_ctr,_sp_sw_lbl[_sp_sw_ctr],_sp_case_ctr)
}

function M_SW_BREAK_Z()
{
	INST3("breq","SW_END_",_sp_sw_ctr,_sp_sw_lbl[_sp_sw_ctr])
}

function M_SW_BREAK_NZ()
{
	INST3("brne","SW_END_",_sp_sw_ctr,_sp_sw_lbl[_sp_sw_ctr])
}

function M_SW_BREAK()
{
	INST3("br","SW_END_",_sp_sw_ctr,_sp_sw_lbl[_sp_sw_ctr])
}

function M_SW_DEFAULT()
{
	LABEL3("SW_",_sp_sw_ctr,_sp_sw_lbl[_sp_sw_ctr],_sp_case_ctr)
}

function M_SW_END()
{
	LABEL3("SW_END_",_sp_sw_ctr,_sp_sw_lbl[_sp_sw_ctr])

	_sp_sw_lbl[_sp_sw_ctr]++
	_sp_case_ctr = POP()
	_sp_sw_ctr--

	# BLOCK BEGIN/END NOT MATCHING
	if (_sp_sw_ctr < 0) ERR_MSG("M_SW_END WITHOUT M_SWITCH")
}


