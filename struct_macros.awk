#----------------------------------------------------------
function ERR_MSG(string)
{
	printf(";MACRO ERROR: %s\n",string)
	printf(";MACRO ERROR AT LINE %05d IN FILE %s: %s\n",_sp_line_number,FILENAME,string) > "/dev/stderr"
}

#function PRINT_STR(STRING)
#{
#	printf("\tcall\tSerialPutImmString\n")
#	printf("\t.asciz\t%s\n",STRING)
#}

function PUSH(VALUE)
{
	_sp_stack[_sp_stack_ptr++] = VALUE
}

function POP(   i)
{
	i = --_sp_stack_ptr
	if (i >= 0)
		return (_sp_stack[i])
	else
		ERR_MSG("PUSH AND POP ARE NOT BALANCED")
}

function SUB(reg,value)
{
	printf ("\tsub\t%s,%s\n",reg,value)
}

function CMP(reg,value)
{
	if (IS_NULL(value))
	{
		SFL(reg)
	} else
	{
		printf ("\tcp\t%s,%s\n",reg,value)
	}
}

function BIT(bit,reg)
{
	printf("\tbit\t%s.%s\n",reg,bit)
}

function ANL(reg,mask)
{
# IF THE MASK ARGUMENT IS SPECIFIED, DO A LOGICAL AND
	if (IS_ARG(mask))
		printf("\tand\t%s,%s\n",reg,mask)
}

function SFL(reg)
{
	printf("\tsfl\t%s\n",reg) 
}

# MACROS NEEDED FOR INTELLIGENT ASSEMBLY
# Constants defined can have these formats:
# 	00h-0ffh
# 	h'00 - h'ff
#	x'00 - x'ff
#	0x'00 - 0x'ff
# 	0x00 - 0xff
#	$00 - $ff

# Returns TRUE if value is 0. This is used when doing a compare with a value: if it's 0, replace
# the instruction CJNE by JNZ or JZ depending of the macro.
function IS_NULL(value)
{
#	result = FALSE
	return (toupper(value) ~ /^#NULL$|^#0X00$|^#0$/)
#	#printf(";IS_NULL: value = %s, match = %d\n",toupper(value),result)
#	return (result)
}

# Return TRUE if there is an argument
function IS_ARG(arg) {return (length(arg))}

# Return TRUE if register specified was not the accumulator
function REG_NOT_ACC(reg) { return (match("A",toupper(reg) ) == 0) }

# Generates a local label
function LOCAL_LABEL()
{
	printf("L_%04d:\n",_sp_label++)
}

# Generates an instruction (jump or similar) to the local label
function INST_LOCAL(inst)
{
	printf("\t%s\tL_%04d\n",inst,_sp_label)
}

# Generates a label with 2 or 3 counters
function LABEL3(label,ctr1,ctr2,ctr3)
{
    if (IS_ARG(ctr3))
    {
        printf("%s%02d%03d%02d:\n",label,ctr1,ctr2,ctr3)
    }
    else
    {
        printf("%s%02d%03d:\n",label,ctr1,ctr2)
    }
}

# Generates an instruction (jump or similar) to a label
function INST(inst,label)
{
    printf("\t%s\t%s\n",inst,label)
}

# Generates an instruction (jump or similar) to a label with 3 counters
function INST3(inst,label,ctr1,ctr2,ctr3)
{
    if (IS_ARG(ctr3))
    {
        printf("\t%s\t%s%02d%03d%02d\n",inst,label,ctr1,ctr2,ctr3)
    }
    else if ((ctr1 == "") && (ctr2 == ""))
    {
        printf("\t%s\t%s\n",inst,label)
    }
    else
    {
       	printf("\t%s\t%s%02d%03d\n",inst,label,ctr1,ctr2)
    }
}
