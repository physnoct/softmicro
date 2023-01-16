#include "softmicro.h"

uint8_t hvect;
uint8_t app_memory[65536];
uint128_t app_reg[16];
uint8_t app_flags;
uint16_t app_pc;
uint8_t app_size,adr_mode;
uint8_t size_byte;
bool step_mode = true;
bool run_until_ret = false;
bool extended = false;

void OpStep(void)
{
uint8_t op_code,param;
int16_t temp;

    app_size = 1;
    adr_mode = 0;
    size_byte = 0;
    extended = false;

    op_code = app_memory[app_pc++];

    if (step_mode) wprintw(wConsole,"OpStep\tSize: %d, Addr mode: %02X, Op: %02X\n",app_size,adr_mode,op_code);

    if (op_code >= 0x40)
    {
        if  (op_code >= 0xE0)
        {
            adr_mode = op_code;
            OpStep3();
        }
        else
        {
            param = op_code & 0x0f;

            switch(op_code & 0xf0)
            {
                case 0x40:
                    app_vector(param);
                    break;
                case 0x50:
                    wait_port(param);
                    break;
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
                case 0xB0:
                    sfl(param);
                    break;
                case 0xC0:
                    push(param);
                    break;
                case 0xD0:
                    pop(param);
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
                break;
            case 0x01: // PUSH PC
                put_retaddr(app_pc-2);
                break;
            case 0x02: // PUSH IMM
                push_imm();
                break;
            case 0x03: // POPN
                popn();
                break;
            case 0x04: //RET
                run_until_ret = false;
                app_pc = get_retaddr();
                break;
            case 0x05: //RETI
                app_flags |= FLAG_I_MASK;
                app_pc = get_retaddr();
                break;
            case 0x06: //PUSHF
                push_byte(app_flags);
                break;
            case 0x07: //POPF
                app_flags = pop_byte();
                break;
            case 0x08: //CALL
                put_retaddr(app_pc + 2); // next instruction after call
                app_pc = getword(app_pc);
                temp = getsp();
                if (step_mode) wprintw(wConsole,"CALL %04X, SP: %04X, Ret Addr %04X\n",app_pc,temp & 0xffff,app_memory[(temp+1) & 0xffff]*256+app_memory[temp & 0xffff]);
                break;
            case 0x09: //BSR
                br_sbr();
                break;
            case 0x0a: //JMP
                app_pc = getword(app_pc);
                if (step_mode) wprintw(wConsole,"JMP %04X\n",app_pc);
                break;
            case 0x0b: //BR
                br_always();
                break;

            /* Size prefix */
            case 0x0c: //.W
                app_size = 2;
                size_byte = 1;
                if (step_mode) wprintw(wConsole,".W\n");
                OpStep2();
                break;
            case 0x0d: //.D
                app_size = 4;
                size_byte = 1;
                if (step_mode) wprintw(wConsole,".D\n");
                OpStep2();
                break;
            case 0x0e: //.Q
                app_size = 8;
                size_byte = 1;
                if (step_mode) wprintw(wConsole,".Q\n");
                OpStep2();
                break;
            case 0x0f: //.O
                app_size = 16;
                size_byte = 1;
                if (step_mode) wprintw(wConsole,".O\n");
                OpStep2();
                break;
            case 0x10: //LD
                op_load();
                break;
            case 0x11: //ADD
                op_add();
                break;
            case 0x12: //ADC
                op_adc();
                break;
            case 0x13: //SUB
                op_sub();
                break;
            case 0x14: //SBC
                op_sbc();
                break;
            case 0x15: //AND
                op_and();
                break;
            case 0x16: //OR
                op_or();
                break;
            case 0x17: //XOR
                op_xor();
                break;
            case 0x18: //CP
                op_cp();
                break;
            case 0x19: //EX
                op_ex();
                break;
            /* Bit on byte ops */
            case 0x1a: // BIT Set/clr
                set_bit();
                break;
            case 0x1b: // BIT toggle
                test_toggle_bit();
                break;
            case 0x1c: // BIT wait
                wait_bit();
                break;
            case 0x1d: // IORES/SET
                { /*
                    uint8_t temp = SoftMicroMemRd(app_pc++);
                    uint8_t port = app_reg[temp & 0x0f].B[0];
                    uint8_t l_bit = (temp >> 4) & 0x07;

                    if ((temp & 0x80) != 0)
                    {
                        SoftMicroIoWr(port, SoftMicroIoRd(port) | (1 << l_bit));
                    }
                    else
                    {
                        SoftMicroIoWr(port, SoftMicroIoRd(port) & ~(1 << l_bit));
                    } */
                }
                break;
            case 0x1e: // IN
                op_in();
                break;
            case 0x1f: // OUT
                op_out();
                break;

            case 0x20: // CLR C
                app_flags &= ~FLAG_C_MASK;
                break;
            case 0x21: // SET C
                app_flags |= FLAG_C_MASK;
                break;
            case 0x22: // TOG C
                app_flags ^= FLAG_C_MASK;
                break;

            /* Unary ops */
            case 0x23: // TOA
                app_reg[0].B[0] = toa_byte(app_reg[0].B[0]);
                break;
            case 0x24: // TOH
                app_reg[0].B[0] = toh_byte(app_reg[0].B[0]);
                break;
            case 0x25: // LDF
                app_reg[0].B[0] = app_flags;
                break;
            case 0x26: // STF
                app_flags = app_reg[0].B[0];
                break;
            case 0x27: // MSK
                op_msk(&app_reg[0]);
                break;
            case 0x28: // SWAP nibbles
                op_swap(&app_reg[0]);
                break;
            case 0x29: // REV
                app_reg[0].B[0] = reverse_byte(app_reg[0].B[0]);
                break;
            case 0x2a: // SXT
                op_sxt();
                break;
            case 0x2b: // CPL
                op_cpl();
                break;
            case 0x2c: // NEG
                op_neg();
                break;

            /* shift/rotate */
            case 0x2d: // ASL/ASR
                op_asx();
                break;
            case 0x2e: // ROL/ROR
                op_rox();
                break;
            case 0x2f: // LSL/LSR
                op_lsx();
                break;

            case 0x30: // Extended
                OpExtended();
                break;
            case 0x31: // BRLT
                branch2(op_code);
                break;
            case 0x32: //BRGE
                branch2(op_code);
                break;
            case 0x33: //DI
                app_flags &= ~FLAG_I_MASK;
                break;
            case 0x34: //EI
                app_flags |= FLAG_I_MASK;
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
                OpStep3();
                break;
            default:
                illegal_inst();
        }
    }
}

/* Ops with size prefix */
void OpStep2(void)
{
uint8_t op_code,param;

    op_code = app_memory[app_pc++];

    if (step_mode) wprintw(wConsole,"OpStep2\tSize: %d, Addr mode: %02X, Op: %02X\n",app_size,adr_mode,op_code);

    if (op_code >= 0x40)
    {
        if  (op_code >= 0xE0)
        {
            /* address prefix */
            adr_mode = op_code;
            OpStep3();
        }
        else
        {
            param = op_code & 0x0f;

            switch(op_code & 0xf0)
            {
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
                case 0xC0:
                    push(param);
                    break;
                case 0xD0:
                    pop(param);
                    break;
                default:
                    illegal_inst();
            }
        }
    }
    else
    {
        switch(op_code)
        {
            case 0x02: // PUSH IMM
                push_imm();
                break;
            case 0x09: //BSR
                br_sbr();
                break;
            case 0x0b: //BR
                br_always();
                break;
            case 0x10: //LD
                op_load();
                break;
            case 0x11: //ADD
                op_add();
                break;
            case 0x12: //ADC
                op_adc();
                break;
            case 0x13: //SUB
                op_sub();
                break;
            case 0x14: //SBC
                op_sbc();
                break;
            case 0x15: //AND
                op_and();
                break;
            case 0x16: //OR
                op_or();
                break;
            case 0x17: //XOR
                op_xor();
                break;
            case 0x18: //CP
                op_cp();
                break;
            case 0x19: //EX
                op_ex();
                break;
            case 0x1e: // IN
                op_in();
                break;
            case 0x1f: // OUT
                op_out();
                break;
            case 0x27: // MSK
                op_msk(&app_reg[0]);
                break;
            case 0x28: // SWAP
                op_swap(&app_reg[0]);
                break;
            case 0x29: // REV
                op_rev();
                break;
            case 0x2a: // SXT
                op_sxt();
                break;
            case 0x2b: // CPL
                op_cpl();
                break;
            case 0x2c: // NEG
                op_neg();
                break;

            /* shift/rotate */
            case 0x2d: // ASL/ASR
                op_asx();
                break;
            case 0x2e: // ROL/ROR
                op_rox();
                break;
            case 0x2f: // LSL/LSR
                op_lsx();
                break;

            case 0x30: // Extended
                OpExtended();
                break;
            case 0x31: //BRLT
                branch2(op_code);
                break;
            case 0x32: //BRGE
                branch2(op_code);
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
                OpStep3();
                break;

            default:
                illegal_inst();
        }
    }
}

/* Ops with address mode prefix */
void OpStep3(void)
{
uint8_t op_code,param;

    op_code = app_memory[app_pc++];

    if (step_mode) wprintw(wConsole,"OpStep3\tSize: %d, Addr mode: %02X, Op: %02X\n",app_size,adr_mode,op_code);

    if (op_code >= 0x70)
    {
        param = op_code & 0x0f;

        switch(op_code & 0xf0)
        {
            case 0x60:
                branch(param);
                break;
            case 0x70:
                djnz(param);
                break;
/*
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
                illegal_inst();
        }
    }
    else
    {
        param = adr_mode & 0x0f;

        switch(op_code)
        {
            case 0x08: //CALL
                ind_call(param);
                break;
            case 0x09: //BSR
                ind_bsr(param);
                break;
            case 0x0a: //JMP
                ind_jmp(param);
                break;
            case 0x10: //LD
                op_load();
                break;
            case 0x11: //ADD
                op_add();
                break;
            case 0x12: //ADC
                op_adc();
                break;
            case 0x13: //SUB
                op_sub();
                break;
            case 0x14: //SBC
                op_sbc();
                break;
            case 0x15: //AND
                op_and();
                break;
            case 0x16: //OR
                op_or();
                break;
            case 0x17: //XOR
                op_xor();
                break;
            case 0x18: //CP
                op_cp();
                break;
            case 0x19: //EX
                op_ex();
                break;
            case 0x1d: // IORES/SET
                switch (adr_mode & 0xf0)
                {
                case 0xe0:
                    op_st();
                    break;
                case 0xf0:
                    { /*
                        uint8_t port = SoftMicroMemRd(app_pc++);
                        uint8_t l_bit = param & 0x07;

                        if ((param & 0x8) != 0)
                        {
                            SoftMicroIoWr(port, SoftMicroIoRd(port) | (1 << l_bit));
                        }
                        else
                        {
                            SoftMicroIoWr(port, SoftMicroIoRd(port) & ~(1 << l_bit));
                        }*/
                    }
                    break;
                default:
                    illegal_inst();
                }
                break;
            case 0x1e: //IN
                op_in();
                break;
            case 0x1f: //OUT
                op_out();
                break;

            case 0x27: //MSK
                op_msk(&app_reg[param]);
                break;
            case 0x28: //SWAP
                op_swap(&app_reg[param]);
                break;
            case 0x29: //REV
                op_rev();
                break;
            case 0x2a: //SXT
                op_sxt();
                break;
            case 0x2b: //CPL
                op_cpl();
                break;
            case 0x2c: //NEG
                op_neg();
                break;
            case 0x3c ... 0x3f:
            case 0x4c ... 0x4f:
                {
                    uint8_t temp = app_memory[app_pc++];
                    if (step_mode) wprintw(wConsole,"Z80 IO inst: (R%d),(R%d),R%d\n", temp >> 4, temp & 0x0f, param);
                }
                break;
            default:
                illegal_inst();
        }
    }
}

void OpExtended(void)
{
uint8_t op_code,param;

    extended = true;

    op_code = app_memory[app_pc++];

    if (step_mode) wprintw(wConsole,"OpExtended\tSize: %d, Addr mode: %02X, Op: %02X\n",app_size,adr_mode,op_code);

    if (op_code >= 0x60)
    {
        param = op_code & 0x0f;

        switch(op_code & 0xf0)
        {
            case 0x60:
                bsr_cond(param);
                break;
            case 0x70:
                ret_cond(param);
                break;
            case 0xD0:
                mswap(param);
                break;
            case 0xE0:
                lddr(param);
                break;
            case 0xF0:
                ldir(param);
                break;
            default:
                illegal_inst();
        }
    }
    else
    {
        switch(op_code)
        {
            case 0x00: //VER
                app_reg[0].B[0] = SOFT_MICRO_INST_SET_VERSION;
                break;
            case 0x01: //SN
                app_reg[0].Q[0] = 0xFEEDBABEDEADBEEF;
                break;
            case 0x02: //HALT
                app_pc-=2;
                wprintw(wConsole,"Halt\n");
                step_mode = true;
                break;
            case 0x03: //CLR H
                app_flags &= ~FLAG_H_MASK;
                break;
            case 0x04: //SET H
                app_flags |= FLAG_H_MASK;
                break;
            case 0x05: //TOG H
                app_flags ^= FLAG_H_MASK;
                break;
            case 0x06: //CLR T
                app_flags &= ~FLAG_T_MASK;
                break;
            case 0x07: //SET T
                app_flags |= FLAG_T_MASK;
                break;
            case 0x08: //TOG T
                app_flags ^= FLAG_T_MASK;
                break;
            case 0x09: //BIN to BCD
                switch(app_size)
                {
                    case 1:
                        app_reg[0].B[0] = bcd_byte(app_reg[0].B[0]);
                        break;
                    case 2:
                        app_reg[0].W[0] = bcd_word(app_reg[0].W[0]);
                        break;
                    case 4:
                        app_reg[0].D[0] = bcd_double(app_reg[0].D[0]);
                        break;
                    case 8:
                        app_reg[0].Q[0] = bcd_quad(app_reg[0].Q[0]);
                        break;
                    case 16:
                        //FIXME to be implemented?
                        break;
                }
                break;
            case 0x0A: //BCD to BIN
                switch(app_size)
                {
                    case 1:
                        app_reg[0].B[0] = bin_byte(app_reg[0].B[0]);
                        break;
                    case 2:
                        app_reg[0].W[0] = bin_word(app_reg[0].W[0]);
                        break;
                    case 4:
                        app_reg[0].D[0] = bin_double(app_reg[0].D[0]);
                        break;
                    case 8:
                        app_reg[0].Q[0] = bin_quad(app_reg[0].Q[0]);
                        break;
                    case 16:
                        //FIXME to be implemented?
                        break;
                }
                break;
            case 0x0B: // RAND
                op_rand();
                break;
            case 0x0F: // STEP
                step_mode = true;
                wprintw(wConsole,"Step\n");
                break;
            case 0x10: //LD
                op_load();
                break;
            case 0x11: //ADD
                op_add();
                break;
            case 0x12: //ADC
                op_adc();
                break;
            case 0x13: //SUB
                op_sub();
                break;
            case 0x14: //SBC
                op_sbc();
                break;
            case 0x15: //AND
                op_and();
                break;
            case 0x16: //OR
                op_or();
                break;
            case 0x17: //XOR
                op_xor();
                break;
            case 0x18: //CP
                op_cp();
                break;
            case 0x19: //EX
                op_ex();
                break;
            case 0x1C: //OUTV
                {
                    uint8_t port = app_memory[app_pc++];
                    uint8_t value = app_memory[app_pc++];
                    if (step_mode)
                    {
                        wprintw(wConsole,"OUT: (%02X) = %02X\n",port,value);
                    }
            	}
                break;
            case 0x1E: //IN
                op_in();
                break;
            case 0x1F: //OUT
                op_out();
                break;
            case 0x20: //VASM
                app_reg[0].B[1] = app_memory[app_pc++];
                wprintw(wConsole,"Assembler instruction set version: %d\n",app_reg[0].B[1]);
                break;
            case 0x21: //MFILL
                mfill();
                break;
            case 0x23: //TOUP
                app_reg[0].B[0] = toupper(app_reg[0].B[0]);
                break;
            case 0x24: //TOLO
                app_reg[0].B[0] = tolower(app_reg[0].B[0]);
                break;
            case 0x25: //LDV
                app_reg[0].B[0] = hvect;
                break;
            case 0x26: //STV
                hvect = app_reg[0].B[0];
                break;
            case 0x27: //UMUL
                op_umul();
                break;
            case 0x28: //UDIV
                op_udiv();
                break;
            case 0x29: //SMUL
                op_smul();
                break;
            case 0x2A: //SDIV
                op_sdiv();
                break;

            default:
                illegal_inst();
        }
    }
}
