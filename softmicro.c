#include "softmicro.h"

uint8_t app_memory[65536];
uint8_t app_reg[16][16];
uint8_t app_flags;
int16_t app_pc;
uint8_t app_size,adr_mode;
bool step_mode = true;

void OpStep(void)
{
uint8_t op_code,param;
int16_t temp;

    app_size = 1;
    adr_mode = 0;

    op_code = app_memory[app_pc++];

    if (step_mode) printw("OpStep\tSize: %d, Addr mode: %02X, Op: %02X\n",app_size,adr_mode,op_code);

    if (op_code >= 0x40)
    {
        if  (op_code >= 0xC0)
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
                case 0xb0:
                    sfl(param);
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
            case 0x01: //BRGE
                branch2(op_code);
                break;
            case 0x02: //DI
                app_flags &= ~FLAG_I_MASK;
                break;
            case 0x03: //EI
                app_flags |= FLAG_I_MASK;
                break;
            case 0x04: //RET
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
                if (step_mode) printw("CALL %04X, SP: %04X, Ret Addr %04X\n",app_pc,temp & 0xffff,app_memory[(temp+1) & 0xffff]*256+app_memory[temp & 0xffff]);
                break;
            case 0x09: //BSR
                br_sbr();
                break;
            case 0x0a: //JMP
                app_pc = getword(app_pc);
                if (step_mode) printw("JMP %04X\n",app_pc);
                break;
            case 0x0b: //BR
                br_always();
                break;

            /* Size prefix */
            case 0x0c: //.W
                app_size = 2;
                if (step_mode) printw(".W\n");
                OpStep2();
                break;
            case 0x0d: //.D
                app_size = 4;
                if (step_mode) printw(".D\n");
                OpStep2();
                break;
            case 0x0e: //.Q
                app_size = 8;
                if (step_mode) printw(".Q\n");
                OpStep2();
                break;
            case 0x0f: //.O
                app_size = 16;
                if (step_mode) printw(".O\n");
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
                toggle_bit();
                break;
            case 0x1c: // BIT test
                test_bit();
                break;
            case 0x1d: // BIT wait
                wait_bit();
                break;
            case 0x1e: // IN
                op_in();
                break;
            case 0x1f: // OUT
                op_out();
                break;

            /* Unary ops */
            case 0x21: // TOA
                app_reg[0][0] = toa_byte(app_reg[0][0]);
                break;
            case 0x22: // TOH
                app_reg[0][0] = toh_byte(app_reg[0][0]);
                break;
            case 0x23: // BCD
                app_reg[0][0] = bcd_byte(app_reg[0][0]);
                break;
            case 0x24: // BIN
                app_reg[0][0] = bin_byte(app_reg[0][0]);
                break;
            case 0x25: // LDF
                app_reg[0][0] = app_flags;
                break;
            case 0x26: // STF
                app_flags = app_reg[0][0];
                break;
            case 0x27: // MSK
                app_reg[0][0] = 1 << (app_reg[0][0] & 0x07);
                break;
            case 0x28: // SWAP
                app_reg[0][0] = swap_byte(app_reg[0][0]);
                break;
            case 0x29: // REV
                app_reg[0][0] = reverse_byte(app_reg[0][0]);
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
            case 0x32: // CLR C
                app_flags &= ~FLAG_C_MASK;
                break;
            case 0x33: // SET C
                app_flags |= FLAG_C_MASK;
                break;
            case 0x34: // TOG C
                app_flags ^= FLAG_C_MASK;
                break;
            case 0x35: // Counter prefix dec
            case 0x36: // Counter prefix inc
                // TBD
                break;
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

    if (step_mode) printw("OpStep2\tSize: %d, Addr mode: %02X, Op: %02X\n",app_size,adr_mode,op_code);

    if (op_code >= 0x40)
    {
        if  (op_code >= 0xC0)
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
                default:
                    illegal_inst();
            }
        }
    }
    else
    {
        switch(op_code)
        {
            case 0x01: //BRGE
                branch2(op_code);
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

            case 0x31: //BRLT
                branch2(op_code);
                break;
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

    if (step_mode) printw("OpStep3\tSize: %d, Addr mode: %02X, Op: %02X\n",app_size,adr_mode,op_code);

    if (op_code >= 0x70)
    {
        if  (op_code >= 0xC0)
        {
            illegal_inst();
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
    }
    else
    {
        switch(op_code)
        {
            case 0x08: //CALL
                //TBD
                break;
            case 0x0a: //JMP
                //TBD
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

            default:
                illegal_inst();
        }
    }
}

void OpExtended(void)
{
uint8_t op_code; //,param;

    op_code = app_memory[app_pc];

    if (step_mode) printw("OpExtended\tSize: %d, Addr mode: %02X, Op: %02X\n",app_size,adr_mode,op_code);

    switch(op_code)
    {
        case 0x00: //VER
            app_reg[0][0] = 1;
            break;
        case 0x01: //SN
            app_reg[0][0] = 0xEF;
            app_reg[0][1] = 0xBE;
            app_reg[0][2] = 0xAD;
            app_reg[0][3] = 0xDE;
            app_reg[0][4] = 0xBE;
            app_reg[0][5] = 0xBA;
            app_reg[0][6] = 0xED;
            app_reg[0][7] = 0xFE;
            break;
        case 0x02: //HALT
            app_pc--;
            printw("Halt\n");
            step_mode = true;
            break;
        case 0x03: // CLR H
            app_flags &= ~FLAG_H_MASK;
            break;
        case 0x04: // SET H
            app_flags |= FLAG_H_MASK;
            break;
        case 0x05: // TOG H
            app_flags ^= FLAG_H_MASK;
            break;
        case 0x06: // CLR T
            app_flags &= ~FLAG_T_MASK;
            break;
        case 0x07: // SET T
            app_flags |= FLAG_T_MASK;
            break;
        case 0x08: // TOG T
            app_flags ^= FLAG_T_MASK;
            break;
        case 0x09: // DAA
            app_reg[0][0] = daa_byte(app_reg[0][0]);
            break;
        case 0x10: // STEP
            step_mode = true;
            printw("Step\n");
            break;

        default:
            illegal_inst();
    }
}
