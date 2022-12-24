/*---------------------------------------------------------------------------*
 * decode.c                                                                  *
 * Copyright (C) 2022  Jacques Pelletier                                     *
 *                                                                           *
 * This program is free software; you can redistribute it and *or            *
 * modify it under the terms of the GNU General Public License               *
 * as published by the Free Software Foundation; either version 2            *
 * of the License, or (at your option) any later version.                    *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program; if not, write to the Free Software Foundation,   *
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.           *
 *---------------------------------------------------------------------------*/

#include "softmicro.h"

char decode_buffer[80];
char dump_buffer[80];
int inst_length;
uint8_t temp;
uint16_t temp16;
char *p_dump;
char *p_decode;

const char flag_table[16][3] = {"CC","CS","NE","EQ","PL","MI","VC","VS","IC","IS","HC","HS","TC","TS","SC","SS"};

char *size_str(int size)
{
	switch(size)
	{
	case 2:	return ".W";
	case 4:	return ".D";
	case 8:	return ".Q";
	case 16: return ".O";
	}
	return "";
}

void DecodeIllegal_inst(void)
{
	sprintf(p_decode, "ILLEGAL");
}

uint8_t decodeGetByte(uint16_t addr)
{
uint8_t data_byte;

	data_byte = app_memory[addr];
	sprintf(p_dump," %02X", data_byte & 0xff);
	inst_length++;
    p_dump += 3;
    return data_byte & 0xff;
}

uint16_t decodeGetWord(uint16_t addr)
{
uint8_t low_byte, high_byte;

	low_byte = app_memory[addr];
	high_byte = app_memory[addr+1];
	sprintf(p_dump," %02X %02X", low_byte, high_byte);
	inst_length += 2;
	p_dump += 6;

    return high_byte*256 + low_byte;
}

void getImmData(uint16_t address)
{
int i;
uint8_t buffer[16];

	//sprintf(p_decode, "@%04X ", address);
	//p_decode += 6;

	// get low endian bytes
	for (i=0;i<app_size;i++)
	{
		buffer[i] = decodeGetByte(address++);
	}

	//print them in big endian
	i = app_size;
	do
	{
		sprintf(p_decode, "%02X", buffer[i-1]);
		p_decode += 2;
	} while (--i);
}

void decodeInstSize(char *Inst)
{
    if (app_size == 1)
    {
        sprintf(p_decode, "%s ",Inst);
        p_decode = p_decode + strlen(Inst) +1;
    }
    else
    {
        sprintf(p_decode, "%s%s ", Inst, size_str(app_size));
        p_decode = p_decode + strlen(Inst) +3;
    }
}

void decodeIndMode(uint16_t address)
{
uint16_t disp16, table, table_offset;
uint8_t disp;
uint8_t reg = adr_mode & 0x0f;

    switch(adr_mode & 0xF0)
    {
        case 0xE0: // (r)
			sprintf(p_decode, "(R%d)", adr_mode & 0x0f);
            break;
        case 0xF0: // (table)[r]
    		switch(app_size)
    		{
				case 2:
					disp16 = decodeGetWord(address);
					sprintf(p_decode, "(%04X)[R%d] ;%04X", disp16, reg, address + ((int16_t) disp16));
					break;
				case 1:
					disp = decodeGetByte(address);
					table = address + 1 + ((int8_t) disp);
					table_offset = get_addr(reg)*2;

					sprintf(p_decode, "(%02X)[R%d]\t;%04X[%04X]: %04X",
						disp,
						reg,
						table,
						get_addr(reg),
						getword(table + table_offset));
					break;
				default:
					DecodeIllegal_inst();
    		}
            break;
        default:
        	DecodeIllegal_inst();
    }
}

void decodeAddressMode(uint16_t address)
{
uint8_t reg;

    reg = adr_mode & 0x0f;

    switch(adr_mode & 0xf0)
    {
        case 0x30:
            switch (adr_mode)
            {
                case 0x35: //R,(~ADDR)[R]   ;relative to PC
    				temp = decodeGetByte(address++);
             		temp16 = decodeGetWord(address);
             		if (app_size == 1)
                    {
                        sprintf(p_decode, "R%d,(~%04X)[R%d] ;%04X", temp >> 4, temp16, temp & 0x0f, address + temp16 - 3);
                    }
                    else
                    {
                        sprintf(p_decode, "R%d,(~%04X)[R%d] ;%04X", temp >> 4, temp16, temp & 0x0f, address + temp16 - 4);
                    }
             		break;

                case 0x36: //R,(ADDR)[R]
    				temp = decodeGetByte(address++);
             		temp16 = decodeGetWord(address);
					sprintf(p_decode, "R%d,(%04X)[R%d]", temp >> 4, temp16, temp & 0x0f);
                    break;

                case 0x37: //(ADDR),#
             		temp16 = decodeGetWord(address);
					sprintf(p_decode, "(%04X),#", temp16);
					p_decode += 8;
					getImmData(address+2);
					break;

                case 0x38: //r,*p
    				temp = decodeGetByte(address);
					sprintf(p_decode, "R%d,*R%d", temp >> 4, temp & 0x0f);
                    break;

                case 0x39: //r,*p++
    				temp = decodeGetByte(address);
					sprintf(p_decode, "R%d,*R%d++", temp >> 4, temp & 0x0f);
                    break;

                case 0x3A: //r,--*p
    				temp = decodeGetByte(address);
					sprintf(p_decode, "R%d,--*R%d", temp >> 4, temp & 0x0f);
                    break;

                case 0x3B: //r,p[d] d: index 0-n
    				temp = decodeGetByte(address++);
    				reg = decodeGetByte(address);
					sprintf(p_decode, "R%d,R%d[%02X]", temp >> 4, temp & 0x0f, reg);
                    break;

                case 0x3C: //*p,r
    				temp = decodeGetByte(address);
					sprintf(p_decode, "*R%d,R%d", temp >> 4, temp & 0x0f);
                    break;

                case 0x3D: //*p++,r
    				temp = decodeGetByte(address);
					sprintf(p_decode, "*R%d++,R%d", temp >> 4, temp & 0x0f);
                    break;

                case 0x3E: //--*p,r
    				temp = decodeGetByte(address);
					sprintf(p_decode, "--*R%d,R%d", temp >> 4, temp & 0x0f);
                    break;

                case 0x3F: //p[d],r d: index 0-n
    				temp = decodeGetByte(address++);
    				reg = decodeGetByte(address);
					sprintf(p_decode, "R%d[%02X],R%d", temp >> 4, reg, temp & 0x0f);
                    break;

                default:
                	DecodeIllegal_inst();
            }
            break;

        case 0xF0: //r,#
        	sprintf(p_decode, "R%d,#", reg);
        	p_decode += 4;
        	if (reg > 9) p_decode++;
        	getImmData(address);
            break;

        default:
        	DecodeIllegal_inst();
    }
}

void OpDecodeExtended(uint16_t address)
{
uint8_t op_code, param;

    op_code = decodeGetByte(address++);

    if (op_code >= 0x60)
    {
        param = op_code & 0x0f;

        switch(op_code & 0xf0)
        {
            case 0x60:
        		switch(app_size)
        		{
					case 2:
						temp16 = decodeGetWord(address);
						sprintf(p_decode, "BSR%s.W %04X ;%04X", flag_table[param], temp16, address + ((int16_t) temp16));
						break;
					case 1:
						temp = decodeGetByte(address);
						sprintf(p_decode, "BSR%s %02X ;%04X", flag_table[param], temp, address + ((int8_t) temp));
						break;
					default:
						DecodeIllegal_inst();
        		}
                break;
            case 0xD0:
            	//swap
            	temp = decodeGetByte(address);	//reg pair
            	sprintf(p_decode, "MSWAP (R%d,R%d),R%d", temp >> 4, temp & 0x0f, param);
                break;
            case 0xE0:
            	//cpdr
            	temp = decodeGetByte(address);	//reg pair
            	sprintf(p_decode, "MCPDR (R%d,R%d),R%d", temp >> 4, temp & 0x0f, param);
                break;
            case 0xF0:
            	//cpir
            	temp = decodeGetByte(address);	//reg pair
            	sprintf(p_decode, "MCPIR (R%d,R%d),R%d", temp >> 4, temp & 0x0f, param);
                break;
            default:
            	DecodeIllegal_inst();
        }
    }
    else
    {
        switch(op_code)
        {
            case 0x00: //VER
            	sprintf(p_decode, "VER");
                break;
            case 0x01: //SN
            	sprintf(p_decode, "SN");
                break;
            case 0x02: //HALT
            	sprintf(p_decode, "HALT");
                break;
            case 0x03: // CLR H
            	sprintf(p_decode, "CLR H");
                break;
            case 0x04: // SET H
            	sprintf(p_decode, "SET H");
                break;
            case 0x05: // TOG H
            	sprintf(p_decode, "TOG H");
                break;
            case 0x06: // CLR T
            	sprintf(p_decode, "CLR T");
                break;
            case 0x07: // SET T
            	sprintf(p_decode, "SET T");
                break;
            case 0x08: // TOG T
            	sprintf(p_decode, "TOG T");
                break;
            case 0x09: // BCD
            	sprintf(p_decode, "BCD");
                break;
            case 0x0A: // BIN
            	sprintf(p_decode, "BIN");
                break;
            case 0x0B: // RAND
            	sprintf(p_decode, "RAND%s", size_str(app_size));
                break;
            case 0x10: // STEP
            	sprintf(p_decode, "STEP");
                break;
            case 0x20: // VASM
            	temp = decodeGetByte(address++);
                sprintf(p_decode, "VASM ;%d", temp);
                break;
            case 0x21: // MFILL
                // end:     end = src  = reg_pair & 0x0f;
                // start: begin = dest = (reg_pair & 0xf0)>>4;
            	temp = decodeGetByte(address++);	//reg pair
            	param = decodeGetByte(address);	//value
            	sprintf(p_decode, "MFILL (R%d,R%d),#%02X", temp >> 4, temp & 0x0f, param);
                break;
            default:
            	DecodeIllegal_inst();
        }
    }
}

/* Ops with address mode prefix */
void OpDecodeStep3(uint16_t address)
{
	uint8_t op_code,param;

    op_code = decodeGetByte(address++);

	if (op_code >= 0x70)
	{
		param = op_code & 0x0f;

		switch(op_code & 0xf0)
		{
/*
			case 0x60:
				branch(param);
				break;
			case 0x70:
				djnz(param);
				break;
			case 0x80:
				clr(param);
				break;
			case 0x90:
				inc(param);
				break;
			case 0xA0:
				dec(param);
				break;
			case 0xb0:
				sfl(param);
				break;
*/
			default:
				DecodeIllegal_inst();
		}
	}
	else
	{
		param = adr_mode & 0x0f;

		switch(op_code)
		{
			case 0x08: //CALL
				sprintf(p_decode, "CALL ");
				p_decode += 5;
				decodeIndMode(address);
				break;
			case 0x09: //BSR
				sprintf(p_decode, "BSR ");
				p_decode += 4;
				decodeIndMode(address);
				break;
			case 0x0a: //JMP
				sprintf(p_decode, "JMP ");
				p_decode += 4;
				decodeIndMode(address);
				break;
			case 0x10: //LD
			    decodeInstSize("LD");
				decodeAddressMode(address);
				break;
			case 0x11: //ADD
			    decodeInstSize("ADD");
				decodeAddressMode(address);
				break;
			case 0x12: //ADC
			    decodeInstSize("ADC");
				decodeAddressMode(address);
				break;
			case 0x13: //SUB
			    decodeInstSize("SUB");
				decodeAddressMode(address);
				break;
			case 0x14: //SBC
			    decodeInstSize("SBC");
				decodeAddressMode(address);
				break;
			case 0x15: //AND
			    decodeInstSize("AND");
				decodeAddressMode(address);
				break;
			case 0x16: //OR
			    decodeInstSize("OR");
				decodeAddressMode(address);
				break;
			case 0x17: //XOR
			    decodeInstSize("XOR");
				decodeAddressMode(address);
				break;
			case 0x18: //CP
			    decodeInstSize("CP");
				decodeAddressMode(address);
				break;
			case 0x19: //EX
			    decodeInstSize("EX");
				decodeAddressMode(address);
				break;
			case 0x1e: // IN
				if ((adr_mode & 0xf0) != 0xf0) DecodeIllegal_inst();
            	temp = decodeGetByte(address);	//reg pair
				sprintf(p_decode, "IN R%d,(%02X)", adr_mode & 0x0f, temp);
				break;
			case 0x1f: // OUT
				if ((adr_mode & 0xf0) != 0xf0) DecodeIllegal_inst();
            	temp = decodeGetByte(address);	//reg pair
				sprintf(p_decode, "OUT (%02X),R%d", temp, adr_mode & 0x0f);
				break;
			case 0x29: // REV
				if ((adr_mode & 0xf0) != 0xf0) DecodeIllegal_inst();
				sprintf(p_decode, "REV%s R%d", size_str(app_size), adr_mode & 0x0f);
				break;
			case 0x2a: // SXT
				if ((adr_mode & 0xf0) != 0xf0) DecodeIllegal_inst();
				sprintf(p_decode, "SXT%s R%d", size_str(app_size), adr_mode & 0x0f);
				break;
			case 0x2b: // CPL
				if ((adr_mode & 0xf0) != 0xf0) DecodeIllegal_inst();
				sprintf(p_decode, "CPL%s R%d", size_str(app_size), adr_mode & 0x0f);
				break;
			case 0x2c: // NEG
				if ((adr_mode & 0xf0) != 0xf0) DecodeIllegal_inst();
				sprintf(p_decode, "NEG%s R%d", size_str(app_size), adr_mode & 0x0f);
				break;
			default:
				DecodeIllegal_inst();
		}
	}
}

/* Ops with size prefix */
void OpDecodeStep2(uint16_t address)
{
uint8_t op_code,param, temp;

    op_code = decodeGetByte(address++);

    if (op_code >= 0x40)
    {
        if  (op_code >= 0xE0)
        {
            /* address prefix */
            adr_mode = op_code;
            OpDecodeStep3(address);
        }
        else
        {
            param = op_code & 0x0f;

            switch(op_code & 0xf0)
            {
                case 0x60:
                	//Branch condition
                	if (app_size != 2) DecodeIllegal_inst();
             		temp16 = decodeGetWord(address);
            		sprintf(p_decode, "BR%s.W %04X ;%04X", flag_table[param], temp16, address + temp16 + 2);
                    break;
                case 0x70:
                	//Decrement and jump if not zero
                	if (app_size != 2) DecodeIllegal_inst();
                	temp16 = decodeGetWord(address);
					sprintf(p_decode, "DJNZ.W R%d,%04X ;%04X", param, temp16, address + temp16 + 2);
                    break;
                case 0x80:
                    //Clear
                	sprintf(p_decode, "CLR%s R%d", size_str(app_size), param);
                    break;
                case 0x90:
                	//Increment
                	sprintf(p_decode, "INC%s R%d", size_str(app_size), param);
                    break;
                case 0xA0:
                	//Decrement
                	sprintf(p_decode, "DEC%s R%d", size_str(app_size), param);
                    break;
                case 0xB0:
                	//Set flags from register
                	sprintf(p_decode, "SFL%s R%d", size_str(app_size), param);
                    break;
                case 0xC0:
                	//Push register
                	sprintf(p_decode, "PUSH%s R%d", size_str(app_size), param);
                    break;
                case 0xD0:
                	//Pop register
                	sprintf(p_decode, "POP%s R%d", size_str(app_size), param);
                    break;
                default:
                	DecodeIllegal_inst();
            }
        }
    }
    else
    {
        switch(op_code)
        {
            case 0x02: // PUSH IMM
            	sprintf(p_decode, "PUSH%s #", size_str(app_size));
            	{
            		int i;
            		uint8_t data_byte[16];

            		for (i=0;i<app_size;i++)
            	    {
            			data_byte[i] = decodeGetByte(address);
            	    }

            		//little endian
            		for (i=app_size; i!=0; --i)
            	    {
            			sprintf(p_decode, "%02X", data_byte[i]);
            	    }
            	}
                break;
            case 0x09: //BSR
            	if (app_size != 2) DecodeIllegal_inst();
         		temp16 = decodeGetWord(address);
        		sprintf(p_decode, "BRS%s.W %04X ;%04X", flag_table[param], temp16, address + temp16 + 2);
                break;
            case 0x0b: //BR
            	if (app_size != 2) DecodeIllegal_inst();
         		temp16 = decodeGetWord(address);
        		sprintf(p_decode, "BR%s.W %04X ;%04X", flag_table[param], temp16, address + temp16 + 2);
                break;
            case 0x10: //LD
            	temp = decodeGetByte(address);
            	sprintf(p_decode, "LD%s R%d,R%d", size_str(app_size), temp >> 4, temp & 0x0f);
                break;
            case 0x11: //ADD
            	temp = decodeGetByte(address);
            	sprintf(p_decode, "ADD%s R%d,R%d", size_str(app_size), temp >> 4, temp & 0x0f);
                break;
            case 0x12: //ADC
            	temp = decodeGetByte(address);
            	sprintf(p_decode, "ADC%s R%d,R%d", size_str(app_size), temp >> 4, temp & 0x0f);
                break;
            case 0x13: //SUB
            	temp = decodeGetByte(address);
            	sprintf(p_decode, "SUB%s R%d,R%d", size_str(app_size), temp >> 4, temp & 0x0f);
                break;
            case 0x14: //SBC
            	temp = decodeGetByte(address);
            	sprintf(p_decode, "SBC%s R%d,R%d", size_str(app_size), temp >> 4, temp & 0x0f);
                break;
            case 0x15: //AND
            	temp = decodeGetByte(address);
            	sprintf(p_decode, "AND%s R%d,R%d", size_str(app_size), temp >> 4, temp & 0x0f);
                break;
            case 0x16: //OR
            	temp = decodeGetByte(address);
            	sprintf(p_decode, "OR%s R%d,R%d", size_str(app_size), temp >> 4, temp & 0x0f);
                break;
            case 0x17: //XOR
            	temp = decodeGetByte(address);
            	sprintf(p_decode, "XOR%s R%d,R%d", size_str(app_size), temp >> 4, temp & 0x0f);
                break;
            case 0x18: //CP
            	temp = decodeGetByte(address);
            	sprintf(p_decode, "CP%s R%d,R%d", size_str(app_size), temp >> 4, temp & 0x0f);
                break;
            case 0x19: //EX
            	temp = decodeGetByte(address);
            	sprintf(p_decode, "EX%s R%d,R%d", size_str(app_size), temp >> 4, temp & 0x0f);
                break;
            case 0x29: // REV
            	sprintf(p_decode, "REV%s R0", size_str(app_size));
                break;
            case 0x2a: // SXT
            	sprintf(p_decode, "SXT%s R0", size_str(app_size));
                break;
            case 0x2b: // CPL
            	sprintf(p_decode, "CPL%s R0", size_str(app_size));
                break;
            case 0x2c: // NEG
            	sprintf(p_decode, "NEG%s R0", size_str(app_size));
                break;

            /* shift/rotate */
            case 0x2d: // ASL/ASR
         		temp = decodeGetByte(address);
         		if (temp & 0x80)
         			sprintf(p_decode, "ASR%s R%d,%d", size_str(app_size), temp & 0x0f, (temp & 0x70) >> 4);
         		else
         			sprintf(p_decode, "ASL%s R%d,%d", size_str(app_size), temp & 0x0f, (temp & 0x70) >> 4);
                break;
            case 0x2e: // ROL/ROR
         		temp = decodeGetByte(address);
         		if (temp & 0x80)
         			sprintf(p_decode, "ROR%s R%d,%d", size_str(app_size), temp & 0x0f, (temp & 0x70) >> 4);
         		else
         			sprintf(p_decode, "ROL%s R%d,%d", size_str(app_size), temp & 0x0f, (temp & 0x70) >> 4);
                break;
            case 0x2f: // LSL/LSR
         		temp = decodeGetByte(address);
         		if (temp & 0x80)
         			sprintf(p_decode, "LSR%s R%d,%d", size_str(app_size), temp & 0x0f, (temp & 0x70) >> 4);
         		else
         			sprintf(p_decode, "LSL%s R%d,%d", size_str(app_size), temp & 0x0f, (temp & 0x70) >> 4);
                break;

            case 0x30: // Extended
                OpDecodeExtended(address);
                break;
            case 0x31: //BRLT
            	if (app_size != 2) DecodeIllegal_inst();
         		temp16 = decodeGetWord(address);
        		sprintf(p_decode, "BRLT.W %04X ;%04X", temp16, address + temp16 + 2);
                break;
            case 0x32: //BRGE
            	if (app_size != 2) DecodeIllegal_inst();
         		temp16 = decodeGetWord(address);
        		sprintf(p_decode, "BRGE.W %04X ;%04X", temp16, address + temp16 + 2);
                break;
            case 0x35:
            case 0x36:
            case 0x37:
            case 0x38:
            case 0x39:
            case 0x3a:
            case 0x3b:
            case 0x3c:
            case 0x3d:
            case 0x3e:
            case 0x3f:
                adr_mode = op_code;
                OpDecodeStep3(address);
                break;

            default:
            	DecodeIllegal_inst();
        }
    }
}

void OpDecode(uint16_t address)
{
	uint8_t op_code,param;
	int16_t temp;

	app_size = 1;
	adr_mode = 0;
    size_byte = 1;
	inst_length = 1;

	p_dump = dump_buffer;
	p_decode = decode_buffer;

	for (int i = 0; i<80; i++)
	{
		dump_buffer[i] = 0;
		decode_buffer[i] = 0;
	}

	op_code = app_memory[address++];
    sprintf(p_dump, "%02X", op_code);
    p_dump += 2;

    if (op_code >= 0x40)
    {
        if  (op_code >= 0xE0)
        {
            adr_mode = op_code;
            OpDecodeStep3(address);
        }
        else
        {
            param = op_code & 0x0f;

            switch(op_code & 0xf0)
            {
                case 0x40:
                	//Vect 0-F
                    sprintf(p_decode, "VECT %01X", param);
                    break;
                case 0x50:
                	//Wait port bit
                	if (param & 0x08)
                		sprintf(p_decode, "WPNZ %d,(%02X)", param & 0x07, decodeGetByte(address));
                	else
                		sprintf(p_decode, "WPZ %d,(%02X)", param & 0x07, decodeGetByte(address));
                    break;
                case 0x60:
                	//Conditional branch
            		temp = decodeGetByte(address);
            		sprintf(p_decode, "BR%s %02X ;%04X", flag_table[param], temp, address + temp + 1);
                	break;
                case 0x70:
					temp = decodeGetByte(address);
					sprintf(p_decode, "DJNZ R%d,%02X ;%04X", param, temp, address + temp + 1);
                    break;
                case 0x80:
                    //Clear
                	sprintf(p_decode, "CLR R%d", param);
                    break;
                case 0x90:
                	//Increment
                	sprintf(p_decode, "INC R%d", param);
                    break;
                case 0xA0:
                	//Decrement
                	sprintf(p_decode, "DEC R%d", param);
                    break;
                case 0xB0:
                	//Set flags from register
                	sprintf(p_decode, "SFL R%d", param);
                    break;
                case 0xC0:
                	//Push register
                	sprintf(p_decode, "PUSH R%d", param);
                    break;
                case 0xD0:
                	//Pop register
                	sprintf(p_decode, "POP R%d", param);
                    break;
                default:
                ;
            }
        }
    }
    else
    {
        switch(op_code)
        {
            case 0x00: //NOP
            	sprintf(p_decode, "NOP");
                break;
            case 0x01: // PUSH PC
            	sprintf(p_decode, "PUSH PC");
                break;
            case 0x02: // PUSH IMM
            	sprintf(p_decode, "PUSH #%02X", decodeGetByte(address));
            	break;
            case 0x03: // POPN
            	sprintf(p_decode, "POPN #%d", decodeGetByte(address));
                break;
            case 0x04: //RET
                run_until_ret = false;
            	sprintf(p_decode, "RET");
                break;
            case 0x05: //RETI
            	sprintf(p_decode, "RETI");
                break;
            case 0x06: //PUSHF
            	sprintf(p_decode, "PUSHF");
                break;
            case 0x07: //POPF
            	sprintf(p_decode, "POPF");
                break;
            case 0x08: //CALL
                sprintf(p_decode, "CALL %04X", decodeGetWord(address));
                break;
            case 0x09: //BSR
        		temp16 = decodeGetByte(address);
        		sprintf(p_decode, "BSR %02X ;%04X", temp16, address + temp16 + 1);
                break;
            case 0x0a: //JMP
                sprintf(p_decode, "JMP %04X", decodeGetWord(address));
                break;
            case 0x0b: //BR
        		temp16 = decodeGetByte(address);
        		sprintf(p_decode, "BR %02X ;%04X", temp16, address + temp16 + 1);
                break;

            /* Size prefix */
            case 0x0c: //.W
                app_size = 2;
                size_byte = 1;
                OpDecodeStep2(address);
                break;
            case 0x0d: //.D
                app_size = 4;
                size_byte = 1;
                OpDecodeStep2(address);
                break;
            case 0x0e: //.Q
                app_size = 8;
                size_byte = 1;
                OpDecodeStep2(address);
                break;
            case 0x0f: //.O
                app_size = 16;
                size_byte = 1;
                OpDecodeStep2(address);
                break;
            case 0x10: //LD
            	temp = decodeGetByte(address);
            	sprintf(p_decode, "LD R%d,R%d", temp >> 4, temp & 0x0f);
                break;
            case 0x11: //ADD
            	temp = decodeGetByte(address);
            	sprintf(p_decode, "ADD R%d,R%d", temp >> 4, temp & 0x0f);
                break;
            case 0x12: //ADC
            	temp = decodeGetByte(address);
            	sprintf(p_decode, "ADC R%d,R%d", temp >> 4, temp & 0x0f);
                break;
            case 0x13: //SUB
            	temp = decodeGetByte(address);
            	sprintf(p_decode, "SUB R%d,R%d", temp >> 4, temp & 0x0f);
                break;
            case 0x14: //SBC
            	temp = decodeGetByte(address);
            	sprintf(p_decode, "SBC R%d,R%d", temp >> 4, temp & 0x0f);
                break;
            case 0x15: //AND
            	temp = decodeGetByte(address);
            	sprintf(p_decode, "AND R%d,R%d", temp >> 4, temp & 0x0f);
                break;
            case 0x16: //OR
            	temp = decodeGetByte(address);
            	sprintf(p_decode, "OR R%d,R%d", temp >> 4, temp & 0x0f);
                break;
            case 0x17: //XOR
            	temp = decodeGetByte(address);
            	sprintf(p_decode, "XOR R%d,R%d", temp >> 4, temp & 0x0f);
                break;
            case 0x18: //CP
            	temp = decodeGetByte(address);
            	sprintf(p_decode, "CP R%d,R%d", temp >> 4, temp & 0x0f);
                break;
            case 0x19: //EX
            	temp = decodeGetByte(address);
            	sprintf(p_decode, "EX R%d,R%d", temp >> 4, temp & 0x0f);
                break;
            /* Bit on byte ops */
            case 0x1a: // BIT Set/clr
            	temp = decodeGetByte(address);
            	if (temp & 0x80)
            		sprintf(p_decode, "BITS R%d.%d", temp & 0x0f, (temp >> 4) & 0x07);
            	else
            		sprintf(p_decode, "BITC R%d.%d", temp & 0x0f, (temp >> 4) & 0x07);
                break;
            case 0x1b: // bit 7 = 0: test, 1: toggle
            	temp = decodeGetByte(address);
            	if (temp & 0x80)
            		sprintf(p_decode, "TOG R%d.%d", temp & 0x0f, (temp >> 4) & 0x07);
            	else
            		sprintf(p_decode, "TEST R%d.%d", temp & 0x0f, (temp >> 4) & 0x07);
                break;
            case 0x1c: // FIXME free opcode
           		sprintf(p_decode, "Unused Op");
                break;
            case 0x1d: // BIT wait
            	temp = decodeGetByte(address);
            	if (temp & 0x80)
            		sprintf(p_decode, "WAITRNZ R%d.%d", temp & 0x0f, (temp >> 4) & 0x07);
            	else
            		sprintf(p_decode, "WAITRZ R%d.%d", temp & 0x0f, (temp >> 4) & 0x07);
                break;
            case 0x1e: // IN
            	temp = decodeGetByte(address);
            	sprintf(p_decode, "IN R0,(%02X)", temp);
                break;
            case 0x1f: // OUT
            	temp = decodeGetByte(address);
            	sprintf(p_decode, "OUT (%02X),R0", temp);
                break;

            case 0x20: // CLR C
           		sprintf(p_decode, "CLR C");
                break;
            case 0x21: // SET C
           		sprintf(p_decode, "SET C");
                break;
            case 0x22: // TOG C
           		sprintf(p_decode, "TOG C");
                break;

            /* Unary ops */
            case 0x23: // TOA
            	sprintf(p_decode, "TOA");
                break;
            case 0x24: // TOH
            	sprintf(p_decode, "TOH");
                break;
            case 0x25: // LDF
            	sprintf(p_decode, "LDF");
                break;
            case 0x26: // STF
            	sprintf(p_decode, "STF");
                break;
            case 0x27: // MSK
            	sprintf(p_decode, "MSK");
                break;
            case 0x28: // SWAP
            	sprintf(p_decode, "SWAP");
                break;
            case 0x29: // REV
            	sprintf(p_decode, "REV");
                break;
            case 0x2a: // SXT
            	sprintf(p_decode, "SXT");
                break;
            case 0x2b: // CPL
            	sprintf(p_decode, "CPL");
                break;
            case 0x2c: // NEG
            	sprintf(p_decode, "NEG");
                break;

            /* shift/rotate */
            case 0x2d: // ASL/ASR
         		temp = decodeGetByte(address);
         		if (temp & 0x80)
         			sprintf(p_decode, "ASR R%d,%d", temp & 0x0f, (temp & 0x70) >> 4);
         		else
         			sprintf(p_decode, "ASL R%d,%d", temp & 0x0f, (temp & 0x70) >> 4);
                break;
            case 0x2e: // ROL/ROR
         		temp = decodeGetByte(address);
         		if (temp & 0x80)
         			sprintf(p_decode, "ROR R%d,%d", temp & 0x0f, (temp & 0x70) >> 4);
         		else
         			sprintf(p_decode, "ROL R%d,%d", temp & 0x0f, (temp & 0x70) >> 4);
                break;
            case 0x2f: // LSL/LSR
         		temp = decodeGetByte(address);
         		if (temp & 0x80)
         			sprintf(p_decode, "LSR R%d,%d", temp & 0x0f, (temp & 0x70) >> 4);
         		else
         			sprintf(p_decode, "LSL R%d,%d", temp & 0x0f, (temp & 0x70) >> 4);
                break;

            case 0x30: // Extended
                OpDecodeExtended(address);
                break;
            case 0x31: //BRLT
         		temp = decodeGetByte(address);
        		sprintf(p_decode, "BRLT %02X ;%04X", temp, address + temp + 2);
                break;
            case 0x32: //BRGE
            	temp = decodeGetByte(address);
        		sprintf(p_decode, "BRGE %02X ;%04X", temp, address + temp + 2);
                break;
            case 0x33: //DI
            	sprintf(p_decode, "DI");
                break;
            case 0x34: //EI
            	sprintf(p_decode, "EI");
                break;
            case 0x35:
            case 0x36:
            case 0x37:
            case 0x38:
            case 0x39:
            case 0x3a:
            case 0x3b:
            case 0x3c:
            case 0x3d:
            case 0x3e:
            case 0x3f:
                adr_mode = op_code;
                OpDecodeStep3(address);
                break;
            default:
            	DecodeIllegal_inst();
        }
    }
}












