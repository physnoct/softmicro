function JEQ(reg,value,address)
{
# IF THE VALUE IS "NULL" OR "00H" REPLACE CJNE WITH JZ

	if (IS_NULL(value))
	{
		SFL(reg)
		INST("breq",address)
	} else
	{
		CJEQ(reg,value,address)
	}
}

function CJNE(reg,value,address)
{
	CMP(reg,value)
	INST("brne\t",address)
}

function CJNE_LOCAL(reg,value)
{
	CMP(reg,value)
	printf("\tbrne\tL_%04d\n",_sp_label)
}

function CJNE3(reg,value,address,ctr1,ctr2,ctr3)
{
	CMP(reg,value)
	INST3("brne\t",address,ctr1,ctr2,ctr3)
}

function CJEQ(reg,value,address)
{
	CMP(reg,value)
	INST("breq\t",address)
}

function CJEQ3(reg,value,address,ctr1,ctr2,ctr3)
{
	CMP(reg,value)
    INST3("breq\t",address,ctr1,ctr2,ctr3)
}

function IN_RANGE3(reg,value1,value2,address,ctr1,ctr2,ctr3)
{
	CMP(reg,value1)
    INST3("brcs\t",address,ctr1,ctr2,ctr3)
	CMP(reg,value2)
    INST3("brcc\t",address,ctr1,ctr2,ctr3)
}

function JNE_LOCAL(reg,value)
{
# IF THE VALUE IS "NULL" OR "00H" REPLACE CJNE WITH JNZ
	if (IS_NULL(value))
	{
		SFL(reg)
		INST_LOCAL("brne\t")
	} else
	{
		CJNE_LOCAL(reg,value)
	}
}

function JNE3(reg,value,address,ctr1,ctr2,ctr3)
{
# IF THE VALUE IS "NULL" OR "00H" REPLACE CJNE WITH JNZ
	if (IS_NULL(value))
	{
		SFL(reg)
		INST3("brne\t",address,ctr1,ctr2,ctr3)
	} else
	{
		CJNE3(reg,value,address,ctr1,ctr2,ctr3)
	}
}

function JEQ3(reg,value,address,ctr1,ctr2,ctr3)
{
# IF THE VALUE IS "NULL" OR "00H" REPLACE CJNE WITH JNZ
	if (IS_NULL(value))
	{
		SFL(reg)
		INST3("breq\t",address,ctr1,ctr2,ctr3)
	} else
	{
		CJEQ3(reg,value,address,ctr1,ctr2,ctr3)
	}
}

function IF_Z_OR_C_ELSE3(address,ctr1,ctr2,ctr3)
{
	INST_LOCAL("breq\t")
	INST3("brcc\t",address,ctr1,ctr2,ctr3)

	LOCAL_LABEL()
}

function IF_NC_AND_NZ_ELSE3(address,ctr1,ctr2,ctr3)
{
	INST3("brcs\t",address,ctr1,ctr2,ctr3)
	INST3("breq\t",address,ctr1,ctr2,ctr3)
}

function JMP_ALWAYS(address,ctr1,ctr2,ctr3)
{
	INST3("br",address,ctr1,ctr2,ctr3)
}

function JMP_BIT3(bit,reg,address,ctr1,ctr2,ctr3)
{
    BIT(bit,reg)
    INST3("brne\t",address,ctr1,ctr2,ctr3)
}

function JMP_NOT_BIT3(bit,reg,address,ctr1,ctr2,ctr3)
{
    BIT(bit,reg)
    INST3("breq\t",address,ctr1,ctr2,ctr3)
}

# Jump to the label if reg is not equal to one of 2 values
function JNE_OR2(reg,value1,value2,label,ctr1,ctr2,ctr3,   temp)
{
	temp = "OR2_" _sp_label

	JEQ(reg,value1,temp)
	JNE3(reg,value2,label,ctr1,ctr2,ctr3)

	# GENERATE LOCAL LABEL
	printf("%s:\n",temp)

	_sp_label++
}

# Jump to the label if reg is not equal to one of 3 values
function JNE_OR3(reg,value1,value2,value3,label,ctr1,ctr2,ctr3,   temp)
{
	temp = "OR3_" _sp_label

	JEQ(reg,value1,temp)
	JEQ(reg,value2,temp)
	JNE3(reg,value3,label,ctr1,ctr2,ctr3)

	# GENERATE LOCAL LABEL
	printf("%s:\n",temp)

	_sp_label++
}

# Jump to the label if reg is not equal to one of 4 values
function JNE_OR4(reg,value1,value2,value3,value4,label,ctr1,ctr2,ctr3,   temp)
{
	temp = "OR4_" _sp_label

	# GENERATE ASSEMBLER CODE
	JEQ(reg,value1,temp)
	JEQ(reg,value2,temp)
	JEQ(reg,value3,temp)
	JNE3(reg,value4,label,ctr1,ctr2,ctr3)

	# GENERATE LOCAL LABEL
	printf("%s:\n",temp)

	_sp_label++
}

# A: R != A
# B: Mask != ""
# C: Value != #0
#    LD  A,REG
#    AND A,MASK
#    CP  A,VAL
#
# A: R == A
# B: Mask != ""
# C: Value != #0
#    AND A,MASK
#    CP  A,VAL
#
# A: R != A
# B: Mask == ""
# C: Value != #0
#    LD  A,REG
#    CP  A,VAL
#
# A: R == A
# B: Mask == ""
# C: Value != #0
#    CP  A,VAL
#
# A: R != A
# B: Mask != ""
# C: Value == #0
#    LD  A,REG
#    AND A,MASK
#
# A: R == A
# B: Mask != ""
# C: Value == #0
#    AND A,MASK
#
# A: R != A
# B: Mask == ""
# C: Value == #0
#    LD  A,REG
#    OR  A,A
#
# A: R == A
# B: Mask == ""
# C: Value == #0
#    OR  A,A
#

function M_SETM_FLAGS(REGISTER,MASK,VALUE)
{
    ANL(REGISTER,MASK)

	if (IS_NULL(VALUE))
	{
      if (!IS_ARG(MASK))
	  {
		SFL(REGISTER)
	  }
    }
    else
    {
	  CMP(REGISTER,VALUE)
    }
}

function M_SET_FLAGS(REGISTER,VALUE)
{
	if (IS_NULL(VALUE))
	{
		SFL(REGISTER)
    }
    else
    {
		CMP(REGISTER,VALUE)
    }
}

# For these 3 macros, here's the implementation:
# M_DO_WHILE(REGISTER,CONDITION,VALUE)
# M_WHILE(REGISTER,CONDITION,VALUE)
# M_IF_CMP(REGISTER,CONDITION,VALUE)

# Condition	   Compare		TRUE IF:
#
# R0 <  R1	R0-R1 <  0			CY
# R0 <= R1 		 0 <= R1-R0 		NC
# R0 >  R1		 0 >  R1-R0		CY
# R0 >= R1	R0-R1 >= 0			NC
#
# R0 <  A		 0 <  A-R0		NC and NZ
# R0 <= A		 0 <= A-R0		NC
# R0 >  A		 0 >  A-R0		CY
# R0 >= A		 0 >= A-R0		CY or Z
#
# A <  R1	 A-R1 <	 0			CY
# A <= R1	 A-R1 <= 0			CY or Z
# A >  R1	 A-R1 >  0			NC and NZ
# A >= R1	 A-R1 >= 0			NC
#
#-----------------------------------------
#					TRUE IF:
# 0 >  X or X <  0				CY
# 0 <= X or X >= 0				NC
# 0 >= X or X <= 0				CY or Z
# 0 <  X or X >  0				NC and NZ
#
#

#----------------------------------------------------------
# register AND value CAN BE ALL AVAILABLE ADDRESSING MODE
# register, DIRECT, IMMEDIATE,INDIRECT
#----------------------------------------------------------
function M_COMPARE_JMP3(register,condition,value,label,ctr1,ctr2,ctr3)
{
	# < OR >=
	if (condition ~ /^<$|^>=$|LT|HE/)
	{
		CMP(register,value)

		# register - value < 0
		if (condition ~ /<|LT/)
		{
			INST3("brcc\t",label,ctr1,ctr2,ctr3)
		}
		# register - value >= 0
		else
		{
			INST3("brcs\t",label,ctr1,ctr2,ctr3)
		}
	}
	else
	{
		# > OR <=
		if (condition ~ /^>$|^<=$|HT|LE/)
		{
			CMP(value,register)

			# 0 <= value - register
			if (condition ~ /<=|LE/)
			{
				INST3("brcs\t",label,ctr1,ctr2,ctr3)
			}
			# 0 > value - register
			else
			{
				INST3("brcc\t",label,ctr1,ctr2,ctr3)
			}
	  	}
	  	else
  		{
			# = OR ==
			if (condition ~ /^=$|^==$|EQ/)
			{
				CJNE3(register,value,label,ctr1,ctr2,ctr3)
			}
			else
			{
				# != OR <>
				if (condition ~ /^!=$|^<>$|NE/)
				{
					CJEQ3(register,value,label,ctr1,ctr2,ctr3)
				}
				else
				{
					ERR_MSG("M_CMP_JMP -- UNKNOWN condition")
				}
			}
		}
	}
}

function M_COMPARE_NJMP3(register,condition,value,label,ctr1,ctr2,ctr3)
{
	# < OR >=
	if (condition ~ /^<$|^>=$|LT|HE/)
	{
		CMP(register,value)

		# register - value < 0
		if (condition ~ /<|LT/)
		{
			INST3("brcs\t",label,ctr1,ctr2,ctr3)
		}
		# register - value >= 0
		else
		{
			INST3("brcc\t",label,ctr1,ctr2,ctr3)
		}
	}
	else
	{
		# > OR <=
		if (condition ~ /^>$|^<=$|HT|LE/)
		{
			CMP(value,register)

			# 0 <= value - register
			if (condition ~ /<=|LE/)
			{
				INST3("brcc\t",label,ctr1,ctr2,ctr3)
			}
			# 0 > value - register
			else
			{
				INST3("brcs\t",label,ctr1,ctr2,ctr3)
			}
	  	}
	  	else
  		{
			# = OR ==
			if (condition ~ /^=$|^==$|EQ/)
			{
	   			CJEQ3(register,value,label,ctr1,ctr2,ctr3)
			}
			else
			{
				# != OR <>
				if (condition ~ /^!=$|^<>$|NE/)
				{
			        CJNE3(register,value,label,ctr1,ctr2,ctr3)
				}
				else
				{
					ERR_MSG("M_COMPARE_NJMP -- UNKNOWN CONDITION")
				}
			}
		}
	}
}

function DEC_JUMP_NZ(REGISTER,TYPE,CTR1,CTR2,CTR3)
{
    printf("\tdjnz\t%s,%s%02d%03d%02d\n",REGISTER,TYPE,CTR1,CTR2,CTR3)
}

