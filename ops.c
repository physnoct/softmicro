#include "softmicro.h"

/* flags STHIVNZC
         *-----*-
*/
void setflags(uint8_t reg[])
{
int i;
uint8_t zero = 0;
uint8_t sign;

    for (i=0;i<app_size;i++)
    {
        zero |= reg[i];
    }
    sign = reg[app_size-1] & 0x80; // sign flag in bit 7 for convenience
    app_flags &= ~(FLAG_S_MASK | FLAG_Z_MASK);     // erase previous flags
    app_flags |= sign;
    if (zero == 0) app_flags |= FLAG_Z_MASK;
}

/* reverse bits MSB <-> LSB */
void op_rev(void)
{
uint8_t reg,temp;
int i;
    reg = adr_mode & 0x0f;

    switch(adr_mode & 0xf0)
    {
        case 0:
        case 0xF0:
            for (i=0;i<app_size;i++)
            {
                temp = reverse_byte(app_reg[reg][(app_size-1)-i]);
                app_reg[reg][(app_size-1)-i] = reverse_byte(app_reg[reg][i]);
                app_reg[reg][i] = temp;
            }
            break;
        default:
            illegal_inst();
    }
}

void op_sxt(void)
{
uint8_t reg;
int i;
    reg = adr_mode & 0x0f;

    switch(adr_mode & 0xf0)
    {
        case 0:
        case 0xF0:
            for (i=0;i<app_size;i++)
            {
                app_reg[reg][i] = sxt_byte();
            }
            break;
        default:
            illegal_inst();
    }
}

void op_cpl(void)
{
uint8_t reg;
int i;
    reg = adr_mode & 0x0f;

    switch(adr_mode & 0xf0)
    {
        case 0:
        case 0xF0:
            for (i=0;i<app_size;i++)
            {
                app_reg[reg][i] = ~app_reg[reg][i];
            }
            break;
        default:
            illegal_inst();
    }
}

void op_neg(void)
{
uint8_t reg;
int i;
    reg = adr_mode & 0x0f;

    switch(adr_mode & 0xf0)
    {
        case 0:
        case 0xF0:
            for (i=0;i<app_size;i++)
            {
                app_reg[reg][i] = ~app_reg[reg][i];
            }

            app_flags |= FLAG_C_MASK;
            for (i=0;i<app_size;i++)
            {
                app_reg[reg][i] = byte_adc(app_reg[reg][i],0);
            }

            break;
        default:
            illegal_inst();
    }
}

void op_load(void)
{
uint8_t reg,disp;
int16_t addr;
int i;

    reg = adr_mode & 0x0f;

    switch(adr_mode & 0xf0)
    {
        case 0:
            getPair();

            for (i=0;i<app_size;i++)
            {
                app_reg[dest][i] = app_reg[src][i];
            }
            break;
        case 0x30:
            switch (adr_mode)
            {
                case 0x35: //R,(PC+DISP)[R]
                    getPair();

                    // address of 1st byte following instruction is the reference (disp = 0)
                    addr = 2 + app_pc + getword(app_pc) + get_addr(src)*app_size;
                    app_pc +=2;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] = app_memory[addr++];
                    }
                    break;

                case 0x36: //R,(ADDR)[R]
                    getPair();
                    addr = getword(app_pc) + get_addr(src)*app_size;
                    app_pc +=2;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] = app_memory[addr++];
                    }
                    break;

                case 0x37: //ADDR,#
                    // Read addr
                    addr = getword(app_pc);
                    app_pc +=2;

                    //Imm to memory
                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] = app_memory[app_pc++];
                    }
                    break;

                case 0x38: //r,*p
                    getPair();
                    addr = get_addr(src);

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] = app_memory[addr++];
                    }
                    break;

                case 0x39: //r,*p++
                    getPair();

                    addr = get_addr(src);

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] = app_memory[addr++];
                    }

                    set_addr(src,addr);
                    break;

                case 0x3A: //r,--*p
                    getPair();

                    addr = get_addr(src) - app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] = app_memory[addr++];
                    }
                    set_addr(src,addr);
                    break;

                case 0x3B: //r,p[d] d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];

                    addr = get_addr(src) + disp * app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] = app_memory[addr++];
                    }
                    break;

                case 0x3C: //*p,r
                    getPair();

                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] = app_reg[src][i];
                    }
                    break;

                case 0x3D: //*p++,r
                    getPair();

                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] = app_reg[src][i];
                    }
                    set_addr(dest,addr);
                    break;

                case 0x3E: //--*p,r
                    getPair();

                    addr = get_addr(dest) - app_size;

                    for (i=0;i<app_size;i++)
                    {
                         app_memory[addr++] = app_reg[src][i];
                    }

                    set_addr(dest,addr);
                    break;

                case 0x3F: //p[d],r d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];

                    addr = get_addr(dest) + disp * app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] = app_reg[src][i];
                    }
                    break;

                default:
                    illegal_inst();
            }
            break;

        case 0xF0: //r,#
            for (i=0;i<app_size;i++)
            {
                app_reg[reg][i] = app_memory[app_pc++];
            }
            break;

        default:
            illegal_inst();
    }
}

int16_t ex_reg_memory(uint16_t addr, uint8_t reg)
{
uint8_t temp;
int i;

    for (i=0;i<app_size;i++)
    {
        temp = app_memory[addr];
        app_memory[addr++] = app_reg[reg][i];
        app_reg[reg][i] = temp;
    }
    return addr;
}

void op_ex(void)
{
uint8_t disp,temp;
int16_t addr;
int i;

    switch(adr_mode & 0xf0)
    {
        case 0:
            getPair();

            for (i=0;i<app_size;i++)
            {
                temp = app_reg[src][i];
                app_reg[src][i] = app_reg[dest][i];
                app_reg[dest][i] = temp;
            }
            break;
        case 0x30:
            switch (adr_mode)
            {
                case 0x38: //r,*p
                    getPair();

                    addr = get_addr(src);
                    ex_reg_memory(addr,dest);
                    break;

                case 0x3c: //*p,r
                    getPair();

                    addr = get_addr(dest);
                    ex_reg_memory(addr,src);
                    break;

                case 0x39: //r,*p++
                    getPair();

                    addr = get_addr(src);
                    addr = ex_reg_memory(addr,dest);
                    set_addr(src,addr);
                    break;

                case 0x3d: //*p++,r
                    getPair();

                    addr = get_addr(dest);
                    addr = ex_reg_memory(addr,src);
                    set_addr(dest,addr);
                    break;

                case 0x3A: //r,--*p
                    getPair();

                    addr = get_addr(src) - app_size;
                    addr = ex_reg_memory(addr,dest);
                    set_addr(src,addr);
                    break;

                case 0x3e: //--*p,r
                    getPair();

                    addr = get_addr(dest) - app_size;
                    addr = ex_reg_memory(addr,src);
                    set_addr(dest,addr);
                    break;

                case 0x3B: //r,p[d] d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];

                    addr = get_addr(src) + disp * app_size;
                    ex_reg_memory(addr,dest);
                    break;

                case 0x3f:
                    getPair();
                    disp = app_memory[app_pc++];

                    addr = get_addr(dest) + disp * app_size;
                    ex_reg_memory(addr,src);
                    break;

                default:
                    illegal_inst();
            }
            break;

        default:
            illegal_inst();
    }
}

void op_add(void)
{
uint8_t reg,disp;
int16_t addr;
int i;

    reg = adr_mode & 0x0f;

    switch(adr_mode & 0xf0)
    {
        case 0:
            getPair();

            app_reg[dest][0] = byte_add(app_reg[dest][0],app_reg[src][0]);

            for (i=1;i<app_size;i++)
            {
                app_reg[dest][i] = byte_adc(app_reg[dest][i],app_reg[src][i]);
            }
            setflags(app_reg[dest]);
            break;
        case 0x30:
            switch (adr_mode)
            {
                case 0x35: //R,(PC+DISP)[R]
                    getPair();

                    // address of 1st byte following instruction is the reference (disp = 0)
                    //addr = 2 + app_pc + getword(app_pc) + get_addr(src)*app_size;
                    disp = getword(app_pc) - app_pc;
                    if (step_mode) wprintw(wConsole,"ADD: base: %04X, disp: %04X, index:%04X\n",app_pc,disp,get_addr(src)*app_size);
                    addr = app_pc + disp + get_addr(src)*app_size;

                    app_pc +=2;

                    app_reg[dest][0] = byte_add(app_reg[dest][0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_adc(app_reg[dest][i],app_memory[addr++]);
                    }
                    break;

                case 0x36: //R,(ADDR)[R]
                    getPair();

                    //addr = getword(app_pc) + get_addr(src)*app_size;
                    addr = getword(app_pc); // + get_addr(src)*app_size;
                    if (step_mode) wprintw(wConsole,"ADD: addr: %04X, disp: %04X\n",addr,get_addr(src)*app_size);
                    addr += get_addr(src)*app_size;

                    app_pc +=2;

                    app_reg[dest][0] = byte_add(app_reg[dest][0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_adc(app_reg[dest][i],app_memory[addr++]);
                    }
                    break;

                case 0x37: //ADDR,#
                    // Read addr
                    addr = getword(app_pc);
                    app_pc +=2;

                    app_memory[addr] = byte_add(app_memory[addr],app_memory[app_pc++]);
                    addr++;

                    //Imm to memory
                    for (i=1;i<app_size;i++)
                    {
                        app_memory[addr] = byte_adc(app_memory[addr],app_memory[app_pc++]);
                        addr++;
                    }
                    break;

                case 0x38: //r,*p
                    getPair();

                    addr = get_addr(src);

                    app_reg[dest][0] = byte_add(app_reg[dest][0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_adc(app_reg[dest][i],app_memory[addr++]);
                    }
                    break;

                case 0x39: //r,*p++
                    getPair();

                    addr = get_addr(src);

                    app_reg[dest][0] = byte_add(app_reg[dest][0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_adc(app_reg[dest][i],app_memory[addr++]);
                    }

                    set_addr(src,addr);
                    break;

                case 0x3A: //r,--*p
                    getPair();

                    addr = get_addr(src) - app_size;

                    app_reg[dest][0] = byte_add(app_reg[dest][0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_adc(app_reg[dest][i],app_memory[addr++]);
                    }
                    set_addr(src,addr);
                    break;

                case 0x3B: //r,p[d] d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];

                    addr = get_addr(src) + disp * app_size;

                    app_reg[dest][0] = byte_add(app_reg[dest][0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_adc(app_reg[dest][i],app_memory[addr++]);
                    }
                    break;

                case 0x3C: //*p,r
                    getPair();
                    addr = get_addr(dest);

                    app_memory[addr] = byte_add(app_memory[addr],app_reg[src][0]);
                    addr++;

                    for (i=1;i<app_size;i++)
                    {
                        app_memory[addr] = byte_adc(app_memory[addr],app_reg[src][i]);
                        addr++;
                    }
                    break;

                case 0x3D: //*p++,r
                    getPair();
                    addr = get_addr(dest);

                    app_memory[addr] = byte_add(app_memory[addr],app_reg[src][0]);
                    addr++;

                    for (i=1;i<app_size;i++)
                    {
                        app_memory[addr] = byte_adc(app_memory[addr],app_reg[src][i]);
                        addr++;
                    }
                    set_addr(dest,addr);
                    break;

                case 0x3E: //--*p,r
                    getPair();
                    addr = get_addr(dest) - app_size;

                    app_memory[addr] = byte_add(app_memory[addr],app_reg[src][0]);
                    addr++;

                    for (i=1;i<app_size;i++)
                    {
                        app_memory[addr] = byte_adc(app_memory[addr],app_reg[src][i]);
                        addr++;
                    }

                    set_addr(dest,addr);
                    break;

                case 0x3F: //p[d],r d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];

                    addr = get_addr(dest) + disp * app_size;

                    app_memory[addr] = byte_add(app_memory[addr],app_reg[src][0]);
                    addr++;

                    for (i=1;i<app_size;i++)
                    {
                        app_memory[addr] = byte_adc(app_memory[addr],app_reg[src][i]);
                        addr++;
                    }
                    break;

                default:
                    illegal_inst();
            }
            break;

        case 0xF0: //r,#
            app_reg[reg][0] = byte_add(app_reg[reg][0],app_memory[app_pc++]);

            for (i=1;i<app_size;i++)
            {
                app_reg[reg][i] = byte_adc(app_reg[reg][i],app_memory[app_pc++]);
            }
            break;

        default:
            illegal_inst();
    }
}

void op_adc(void)
{
uint8_t reg,disp;
int16_t addr;
int i;

    reg = adr_mode & 0x0f;

    switch(adr_mode & 0xf0)
    {
        case 0:
            getPair();

            for (i=0;i<app_size;i++)
            {
                app_reg[dest][i] = byte_adc(app_reg[dest][i],app_reg[src][i]);
            }
            setflags(app_reg[dest]);
            break;
        case 0x30:
            switch (adr_mode)
            {
                case 0x35: //R,(PC+DISP)[R]
                    getPair();

                    // address of 1st byte following instruction is the reference (disp = 0)
                    addr = 2 + app_pc + getword(app_pc) + get_addr(src)*app_size;
                    app_pc +=2;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_adc(app_reg[dest][i],app_memory[addr++]);
                    }
                    break;

                case 0x36: //R,(ADDR)[R]
                    getPair();
                    addr = getword(app_pc) + get_addr(src)*app_size;
                    app_pc +=2;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_adc(app_reg[dest][i],app_memory[addr++]);
                    }
                    break;

                case 0x37: //ADDR,#
                    // Read addr
                    addr = getword(app_pc);
                    app_pc +=2;

                    //Imm to memory
                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr] = byte_adc(app_memory[addr],app_memory[app_pc++]);
                        addr++;
                    }
                    break;

                case 0x38: //r,*p
                    getPair();
                    addr = get_addr(src);

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_adc(app_reg[dest][i],app_memory[addr++]);
                    }
                    break;

                case 0x39: //r,*p++
                    getPair();
                    addr = get_addr(src);

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_adc(app_reg[dest][i],app_memory[addr++]);
                    }

                    set_addr(src,addr);
                    break;

                case 0x3A: //r,--*p
                    getPair();
                    addr = get_addr(src) - app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_adc(app_reg[dest][i],app_memory[addr++]);
                    }
                    set_addr(src,addr);
                    break;

                case 0x3B: //r,p[d] d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];

                    addr = get_addr(src) + disp * app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_adc(app_reg[dest][i],app_memory[addr++]);
                    }
                    break;

                case 0x3C: //*p,r
                    getPair();
                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr] = byte_adc(app_memory[addr],app_reg[src][i]);
                        addr++;
                    }
                    break;

                case 0x3D: //*p++,r
                    getPair();
                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr] = byte_adc(app_memory[addr],app_reg[src][i]);
                        addr++;
                    }
                    set_addr(dest,addr);
                    break;

                case 0x3E: //--*p,r
                    getPair();
                    addr = get_addr(dest) - app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr] = byte_adc(app_memory[addr],app_reg[src][i]);
                        addr++;
                    }

                    set_addr(dest,addr);
                    break;

                case 0x3F: //p[d],r d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];

                    addr = get_addr(dest) + disp * app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr] = byte_adc(app_memory[addr],app_reg[src][i]);
                        addr++;
                    }
                    break;

                default:
                    illegal_inst();
            }
            break;

        case 0xF0: //r,#
            for (i=0;i<app_size;i++)
            {
                app_reg[reg][i] = byte_adc(app_reg[reg][i],app_memory[app_pc++]);
            }
            break;

        default:
            illegal_inst();
    }
}

void op_sub(void)
{
uint8_t reg,disp;
int16_t addr;
int i;

    reg = adr_mode & 0x0f;

    switch(adr_mode & 0xf0)
    {
        case 0:
            getPair();

            app_reg[dest][0] = byte_sub(app_reg[dest][0],app_reg[src][0]);

            for (i=1;i<app_size;i++)
            {
                app_reg[dest][i] = byte_sbc(app_reg[dest][i],app_reg[src][i]);
            }
            setflags(app_reg[dest]);
            break;
        case 0x30:
            switch (adr_mode)
            {
                case 0x35: //R,(PC+DISP)[R]
                    getPair();

                    // address of 1st byte following instruction is the reference (disp = 0)
                    addr = 2 + app_pc + getword(app_pc) + get_addr(src)*app_size;
                    app_pc +=2;

                    app_reg[dest][0] = byte_sub(app_reg[dest][0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_sbc(app_reg[dest][i],app_memory[addr++]);
                    }
                    break;

                case 0x36: //R,(ADDR)[R]
                    getPair();
                    addr = getword(app_pc) + get_addr(src)*app_size;
                    app_pc +=2;

                    app_reg[dest][0] = byte_sub(app_reg[dest][0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_sbc(app_reg[dest][i],app_memory[addr++]);
                    }
                    break;

                case 0x37: //ADDR,#
                    // Read addr
                    addr = getword(app_pc);
                    app_pc +=2;

                    app_memory[addr] = byte_sub(app_memory[addr],app_memory[app_pc++]);
                    addr++;

                    //Imm to memory
                    for (i=1;i<app_size;i++)
                    {
                        app_memory[addr] = byte_sbc(app_memory[addr],app_memory[app_pc++]);
                        addr++;
                    }
                    break;

                case 0x38: //r,*p
                    getPair();
                    addr = get_addr(src);

                    app_reg[dest][0] = byte_sub(app_reg[dest][0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_sbc(app_reg[dest][i],app_memory[addr++]);
                    }
                    break;

                case 0x39: //r,*p++
                    getPair();
                    addr = get_addr(src);

                    app_reg[dest][0] = byte_sub(app_reg[dest][0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_sbc(app_reg[dest][i],app_memory[addr++]);
                    }

                    set_addr(src,addr);
                    break;

                case 0x3A: //r,--*p
                    getPair();
                    addr = get_addr(src) - app_size;

                    app_reg[dest][0] = byte_sub(app_reg[dest][0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_sbc(app_reg[dest][i],app_memory[addr++]);
                    }
                    set_addr(src,addr);
                    break;

                case 0x3B: //r,p[d] d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];

                    addr = get_addr(src) + disp * app_size;

                    app_reg[dest][0] = byte_sub(app_reg[dest][0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_sbc(app_reg[dest][i],app_memory[addr++]);
                    }
                    break;

                case 0x3C: //*p,r
                    getPair();
                    addr = get_addr(dest);

                    app_memory[addr] = byte_sub(app_memory[addr],app_reg[src][0]);
                    addr++;

                    for (i=1;i<app_size;i++)
                    {
                        app_memory[addr] = byte_sbc(app_memory[addr],app_reg[src][i]);
                        addr++;
                    }
                    break;

                case 0x3D: //*p++,r
                    getPair();
                    addr = get_addr(dest);

                    app_memory[addr] = byte_sub(app_memory[addr],app_reg[src][0]);
                    addr++;

                    for (i=1;i<app_size;i++)
                    {
                        app_memory[addr] = byte_sbc(app_memory[addr],app_reg[src][i]);
                        addr++;
                    }
                    set_addr(dest,addr);
                    break;

                case 0x3E: //--*p,r
                    getPair();
                    addr = get_addr(dest) - app_size;

                    app_memory[addr] = byte_sub(app_memory[addr],app_reg[src][0]);
                    addr++;

                    for (i=1;i<app_size;i++)
                    {
                        app_memory[addr] = byte_sbc(app_memory[addr],app_reg[src][i]);
                        addr++;
                    }

                    set_addr(dest,addr);
                    break;

                case 0x3F: //p[d],r d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];

                    addr = get_addr(dest) + disp * app_size;

                    app_memory[addr] = byte_sub(app_memory[addr],app_reg[src][0]);
                    addr++;

                    for (i=1;i<app_size;i++)
                    {
                        app_memory[addr] = byte_sbc(app_memory[addr],app_reg[src][i]);
                        addr++;
                    }
                    break;

                default:
                    illegal_inst();
            }
            break;

        case 0xF0: //r,#
            app_reg[reg][0] = byte_sub(app_reg[reg][0],app_memory[app_pc++]);

            for (i=1;i<app_size;i++)
            {
                app_reg[reg][i] = byte_sbc(app_reg[reg][i],app_memory[app_pc++]);
            }
            break;

        default:
            illegal_inst();
    }
}

void op_sbc(void)
{
uint8_t reg,disp;
int16_t addr;
int i;

    reg = adr_mode & 0x0f;

    switch(adr_mode & 0xf0)
    {
        case 0:
            getPair();

            for (i=0;i<app_size;i++)
            {
                app_reg[dest][i] = byte_sbc(app_reg[dest][i],app_reg[src][i]);
            }
            setflags(app_reg[dest]);
            break;
        case 0x30:
            switch (adr_mode)
            {
                case 0x35: //R,(PC+DISP)[R]
                    getPair();

                    // address of 1st byte following instruction is the reference (disp = 0)
                    addr = 2 + app_pc + getword(app_pc) + get_addr(src)*app_size;
                    app_pc +=2;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_sbc(app_reg[dest][i],app_memory[addr++]);
                    }
                    break;

                case 0x36: //R,(ADDR)[R]
                    getPair();

                    addr = getword(app_pc) + get_addr(src)*app_size;
                    app_pc +=2;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_sbc(app_reg[dest][i],app_memory[addr++]);
                    }
                    break;

                case 0x37: //ADDR,#
                    // Read addr
                    addr = getword(app_pc);
                    app_pc +=2;

                    //Imm to memory
                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr] = byte_sbc(app_memory[addr],app_memory[app_pc++]);
                        addr++;
                    }
                    break;

                case 0x38: //r,*p
                    getPair();
                    addr = get_addr(src);

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_sbc(app_reg[dest][i],app_memory[addr++]);
                    }
                    break;

                case 0x39: //r,*p++
                    getPair();
                    addr = get_addr(src);

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_sbc(app_reg[dest][i],app_memory[addr++]);
                    }

                    set_addr(src,addr);
                    break;

                case 0x3A: //r,--*p
                    getPair();
                    addr = get_addr(src) - app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_sbc(app_reg[dest][i],app_memory[addr++]);
                    }
                    set_addr(src,addr);
                    break;

                case 0x3B: //r,p[d] d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];

                    addr = get_addr(src) + disp * app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_sbc(app_reg[dest][i],app_memory[addr++]);
                    }
                    break;

                case 0x3C: //*p,r
                    getPair();
                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr] = byte_sbc(app_memory[addr],app_reg[src][i]);
                        addr++;
                    }
                    break;

                case 0x3D: //*p++,r
                    getPair();
                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr] = byte_sbc(app_memory[addr],app_reg[src][i]);
                        addr++;
                    }
                    set_addr(dest,addr);
                    break;

                case 0x3E: //--*p,r
                    getPair();
                    addr = get_addr(dest) - app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr] = byte_sbc(app_memory[addr],app_reg[src][i]);
                        addr++;
                    }

                    set_addr(dest,addr);
                    break;

                case 0x3F: //p[d],r d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];

                    addr = get_addr(dest) + disp * app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr] = byte_sbc(app_memory[addr],app_reg[src][i]);
                        addr++;
                    }
                    break;

                default:
                    illegal_inst();
            }
            break;

        case 0xF0: //r,#
            for (i=0;i<app_size;i++)
            {
                app_reg[reg][i] = byte_sbc(app_reg[reg][i],app_memory[app_pc++]);
            }
            break;

        default:
            illegal_inst();
    }
}

void op_cp(void)
{
uint8_t reg,disp;
int16_t addr;
int i;

    app_flags |= FLAG_Z_MASK;

    reg = adr_mode & 0x0f;

    switch(adr_mode & 0xf0)
    {
        case 0:
            getPair();

            byte_sub(app_reg[dest][0],app_reg[src][0]);

            for (i=1;i<app_size;i++)
            {
                byte_sbc(app_reg[dest][i],app_reg[src][i]);
            }
            break;
        case 0x30:
            switch (adr_mode)
            {
                case 0x35: //R,(PC+DISP)[R]
                    getPair();

                    // address of 1st byte following instruction is the reference (disp = 0)
                    addr = 2 + app_pc + getword(app_pc) + get_addr(src)*app_size;
                    app_pc +=2;

                    byte_sub(app_reg[dest][0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        byte_sbc(app_reg[dest][i],app_memory[addr++]);
                    }
                    break;

                case 0x36: //R,(ADDR)[R]
                    getPair();
                    addr = getword(app_pc) + get_addr(src)*app_size;
                    app_pc +=2;

                    byte_sub(app_reg[dest][0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        byte_sbc(app_reg[dest][i],app_memory[addr++]);
                    }
                    break;

                case 0x37: //ADDR,#
                    // Read addr
                    addr = getword(app_pc);
                    app_pc +=2;

                    byte_sub(app_memory[addr++],app_memory[app_pc++]);

                    //Imm to memory
                    for (i=1;i<app_size;i++)
                    {
                        byte_sbc(app_memory[addr++],app_memory[app_pc++]);
                    }
                    break;

                case 0x38: //r,*p
                    getPair();
                    addr = get_addr(src);

                    byte_sub(app_reg[dest][0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        byte_sbc(app_reg[dest][i],app_memory[addr++]);
                    }
                    break;

                case 0x39: //r,*p++
                    getPair();
                    addr = get_addr(src);

                    byte_sub(app_reg[dest][0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        byte_sbc(app_reg[dest][i],app_memory[addr++]);
                    }

                    set_addr(src,addr);
                    break;

                case 0x3A: //r,--*p
                    getPair();
                    addr = get_addr(src) - app_size;

                    byte_sub(app_reg[dest][0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        byte_sbc(app_reg[dest][i],app_memory[addr++]);
                    }
                    set_addr(src,addr);
                    break;

                case 0x3B: //r,p[d] d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];

                    addr = get_addr(src) + disp * app_size;

                    byte_sub(app_reg[dest][0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        byte_sbc(app_reg[dest][i],app_memory[addr++]);
                    }
                    break;

                case 0x3C: //*p,r
                    getPair();
                    addr = get_addr(dest);

                    byte_sub(app_memory[addr++],app_reg[src][0]);

                    for (i=1;i<app_size;i++)
                    {
                        byte_sbc(app_memory[addr++],app_reg[src][i]);
                    }
                    break;

                case 0x3D: //*p++,r
                    getPair();
                    addr = get_addr(dest);

                    byte_sub(app_memory[addr++],app_reg[src][0]);

                    for (i=1;i<app_size;i++)
                    {
                        byte_sbc(app_memory[addr++],app_reg[src][i]);
                    }
                    set_addr(dest,addr);
                    break;

                case 0x3E: //--*p,r
                    getPair();
                    addr = get_addr(dest) - app_size;

                    byte_sub(app_memory[addr++],app_reg[src][0]);

                    for (i=1;i<app_size;i++)
                    {
                        byte_sbc(app_memory[addr++],app_reg[src][i]);
                    }

                    set_addr(dest,addr);
                    break;

                case 0x3F: //p[d],r d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];

                    addr = get_addr(dest) + disp * app_size;

                    byte_sub(app_memory[addr++],app_reg[src][0]);

                    for (i=1;i<app_size;i++)
                    {
                        byte_sbc(app_memory[addr++],app_reg[src][i]);
                    }
                    break;

                default:
                    illegal_inst();
            }
            break;

        case 0xF0: //r,#
            byte_sub(app_reg[reg][0],app_memory[app_pc++]);

            for (i=1;i<app_size;i++)
            {
                byte_sbc(app_reg[reg][i],app_memory[app_pc++]);
            }
            break;

        default:
            illegal_inst();
    }
}

void op_and(void)
{
uint8_t reg,disp;
int16_t addr;
int i;

    reg = adr_mode & 0x0f;

    switch(adr_mode & 0xf0)
    {
        case 0:
            getPair();

            for (i=0;i<app_size;i++)
            {
                app_reg[dest][i] &= app_reg[src][i];
            }
            setflags(app_reg[dest]);
            break;
        case 0x30:
            switch (adr_mode)
            {
                case 0x35: //R,(PC+DISP)[R]
                    getPair();

                    // address of 1st byte following instruction is the reference (disp = 0)
                    addr = 2 + app_pc + getword(app_pc) + get_addr(src)*app_size;
                    app_pc +=2;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] &= app_memory[addr++];
                    }
                    break;

                case 0x36: //R,(ADDR)[R]
                    getPair();
                    addr = getword(app_pc) + get_addr(src)*app_size;
                    app_pc +=2;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] &= app_memory[addr++];
                    }
                    break;

                case 0x37: //ADDR,#
                    // Read addr
                    addr = getword(app_pc);
                    app_pc +=2;

                    //Imm to memory
                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] &= app_memory[app_pc++];
                    }
                    break;

                case 0x38: //r,*p
                    getPair();
                    addr = get_addr(src);

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] &= app_memory[addr++];
                    }
                    break;

                case 0x39: //r,*p++
                    getPair();
                    addr = get_addr(src);

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] &= app_memory[addr++];
                    }

                    set_addr(src,addr);
                    break;

                case 0x3A: //r,--*p
                    getPair();
                    addr = get_addr(src) - app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] &= app_memory[addr++];
                    }
                    set_addr(src,addr);
                    break;

                case 0x3B: //r,p[d] d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];

                    addr = get_addr(src) + disp * app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] &= app_memory[addr++];
                    }
                    break;

                case 0x3C: //*p,r
                    getPair();
                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] &= app_reg[src][i];
                    }
                    break;

                case 0x3D: //*p++,r
                    getPair();
                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] &= app_reg[src][i];
                    }
                    set_addr(dest,addr);
                    break;

                case 0x3E: //--*p,r
                    getPair();
                    addr = get_addr(dest) - app_size;

                    for (i=0;i<app_size;i++)
                    {
                         app_memory[addr++] &= app_reg[src][i];
                    }

                    set_addr(dest,addr);
                    break;

                case 0x3F: //p[d],r d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];

                    addr = get_addr(dest) + disp * app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] &= app_reg[src][i];
                    }
                    break;

                default:
                    illegal_inst();
            }
            break;

        case 0xF0: //r,#
            for (i=0;i<app_size;i++)
            {
                app_reg[reg][i] &= app_memory[app_pc++];
            }
            break;

        default:
            illegal_inst();
    }
}

void op_or(void)
{
uint8_t reg,disp;
int16_t addr;
int i;

    reg = adr_mode & 0x0f;

    switch(adr_mode & 0xf0)
    {
        case 0:
            getPair();

            for (i=0;i<app_size;i++)
            {
                app_reg[dest][i] |= app_reg[src][i];
            }
            setflags(app_reg[dest]);
            break;
        case 0x30:
            switch (adr_mode)
            {
                case 0x35: //R,(PC+DISP)[R]
                    getPair();

                    // address of 1st byte following instruction is the reference (disp = 0)
                    addr = 2 + app_pc + getword(app_pc) + get_addr(src)*app_size;
                    app_pc +=2;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] |= app_memory[addr++];
                    }
                    break;

                case 0x36: //R,(ADDR)[R]
                    getPair();
                    addr = getword(app_pc) + get_addr(src)*app_size;
                    app_pc +=2;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] |= app_memory[addr++];
                    }
                    break;

                case 0x37: //ADDR,#
                    // Read addr
                    addr = getword(app_pc);
                    app_pc +=2;

                    //Imm to memory
                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] |= app_memory[app_pc++];
                    }
                    break;

                case 0x38: //r,*p
                    getPair();
                    addr = get_addr(src);

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] |= app_memory[addr++];
                    }
                    break;

                case 0x39: //r,*p++
                    getPair();
                    addr = get_addr(src);

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] |= app_memory[addr++];
                    }

                    set_addr(src,addr);
                    break;

                case 0x3A: //r,--*p
                    getPair();
                    addr = get_addr(src) - app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] |= app_memory[addr++];
                    }
                    set_addr(src,addr);
                    break;

                case 0x3B: //r,p[d] d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];
                    addr = get_addr(src) + disp * app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] |= app_memory[addr++];
                    }
                    break;

                case 0x3C: //*p,r
                    getPair();
                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] |= app_reg[src][i];
                    }
                    break;

                case 0x3D: //*p++,r
                    getPair();
                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] |= app_reg[src][i];
                    }
                    set_addr(dest,addr);
                    break;

                case 0x3E: //--*p,r
                    getPair();
                    addr = get_addr(dest) - app_size;

                    for (i=0;i<app_size;i++)
                    {
                         app_memory[addr++] |= app_reg[src][i];
                    }

                    set_addr(dest,addr);
                    break;

                case 0x3F: //p[d],r d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];

                    addr = get_addr(dest) + disp * app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] |= app_reg[src][i];
                    }
                    break;

                default:
                    illegal_inst();
            }
            break;

        case 0xF0: //r,#
            for (i=0;i<app_size;i++)
            {
                app_reg[reg][i] |= app_memory[app_pc++];
            }
            break;

        default:
            illegal_inst();
    }
}

void op_xor(void)
{
uint8_t reg,disp;
int16_t addr;
int i;

    reg = adr_mode & 0x0f;

    switch(adr_mode & 0xf0)
    {
        case 0:
            getPair();

            for (i=0;i<app_size;i++)
            {
                app_reg[dest][i] ^= app_reg[src][i];
            }
            setflags(app_reg[dest]);
            break;
        case 0x30:
            switch (adr_mode)
            {
                case 0x35: //R,(PC+DISP)[R]
                    getPair();

                    // address of 1st byte following instruction is the reference (disp = 0)
                    addr = 2 + app_pc + getword(app_pc) + get_addr(src)*app_size;
                    app_pc +=2;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] ^= app_memory[addr++];
                    }
                    break;

                case 0x36: //R,(ADDR)[R]
                    getPair();
                    addr = getword(app_pc) + get_addr(src)*app_size;
                    app_pc +=2;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] ^= app_memory[addr++];
                    }
                    break;

                case 0x37: //ADDR,#
                    // Read addr
                    addr = getword(app_pc);
                    app_pc +=2;

                    //Imm to memory
                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] ^= app_memory[app_pc++];
                    }
                    break;

                case 0x38: //r,*p
                    getPair();
                    addr = get_addr(src);

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] ^= app_memory[addr++];
                    }
                    break;

                case 0x39: //r,*p++
                    getPair();
                    addr = get_addr(src);

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] ^= app_memory[addr++];
                    }

                    set_addr(src,addr);
                    break;

                case 0x3A: //r,--*p
                    getPair();
                    addr = get_addr(src) - app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] ^= app_memory[addr++];
                    }
                    set_addr(src,addr);
                    break;

                case 0x3B: //r,p[d] d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];
                    addr = get_addr(src) + disp * app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] ^= app_memory[addr++];
                    }
                    break;

                case 0x3C: //*p,r
                    getPair();
                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] ^= app_reg[src][i];
                    }
                    break;

                case 0x3D: //*p++,r
                    getPair();
                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] ^= app_reg[src][i];
                    }
                    set_addr(dest,addr);
                    break;

                case 0x3E: //--*p,r
                    getPair();
                    addr = get_addr(dest) - app_size;

                    for (i=0;i<app_size;i++)
                    {
                         app_memory[addr++] ^= app_reg[src][i];
                    }

                    set_addr(dest,addr);
                    break;

                case 0x3F: //p[d],r d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];

                    addr = get_addr(dest) + disp * app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] ^= app_reg[src][i];
                    }
                    break;

                default:
                    illegal_inst();
            }
            break;

        case 0xF0: //r,#
            for (i=0;i<app_size;i++)
            {
                app_reg[reg][i] ^= app_memory[app_pc++];
            }
            break;

        default:
            illegal_inst();
    }
}

/* flags STHIVNZC
         *-----**
*/
void op_inc(uint8_t reg[])
{
int i;

    reg[0] = byte_add(reg[0],1);

    for (i=1;i<app_size;i++)
    {
        reg[i] = byte_adc(reg[i],0);
    }
    setflags(reg);
}

void op_dec(uint8_t reg[])
{
int i;

    reg[0] = byte_sub(reg[0],1);

    for (i=1;i<app_size;i++)
    {
        reg[i] = byte_sbc(reg[i],0);
    }
    setflags(reg);
}

void op_in(void)
{
uint8_t port;
int result, val;

    port = app_memory[app_pc++];
    if (step_mode) wprintw(wConsole,"IN: [%02X] = ",port);

    result = scanw("%02X",&val);
    if (step_mode) wprintw(wConsole,"Result: %02X, Input: %02X\n",result,val & 0xff);

    if ((adr_mode & 0xf0) == 0xF0)
    {
        app_reg[adr_mode & 0xf][0] = val & 0xff;
    }
    else
    {
        app_reg[0][0] = val & 0xff;
    }
}

void op_out(void)
{
uint8_t port,val;
    port = app_memory[app_pc++];

    if ((adr_mode & 0xf0) == 0xF0)
    {
        val = app_reg[adr_mode & 0xf][0];
    }
    else
    {
        val = app_reg[0][0];
    }
    if (step_mode)
    {
        wprintw(wConsole,"OUT: [%02X] = %02X\n",port,val);
    }
    else
    {
        if (port == 0xFF)
        {
            addch(val);
            refresh();
        }
    }
}

/*
Shift ops
bit 7       0: left     1: right
bits 6-4    nb of shift
bits 3-0    reg
*/
/* S -> bits -> cy */
/* cy <- bits <- 0 */
void op_asx(void)
{
uint8_t param;
uint8_t nb_shifts,reg,side;
int i,j;
int temp, temp_cy;

    param = app_memory[app_pc++];
    side = param & 0x80;
    nb_shifts = (param & 0x70) >> 4;
    reg = param & 0x0f;

    if (step_mode) wprintw(wConsole,"ASX Side: %d, Shifts: %d, Reg: %d\n",side,nb_shifts,reg);

    if (side == 0)
    {
        /* left */
        for (i=0;i<nb_shifts;i++)
        {
            /* Clear carry flag */
            app_flags &= ~FLAG_C_MASK;

            for (j=0;j<app_size;j++)
            {
                temp = (app_reg[reg][j] << 1) | (app_flags & FLAG_C_MASK);
                app_flags = ((temp & 0x100) >> 8) | (app_flags & ~FLAG_C_MASK);
                app_reg[reg][j] = temp & 0xff;
            }
        }
    }
    else
    {
        /* right */
        for (i=0;i<nb_shifts;i++)
        {
            /* Get sign bit */
            app_flags = (app_flags & ~FLAG_C_MASK) | ((app_reg[reg][app_size-1] & 0x80) >> 7);

            for (j=0;j<app_size;j++)
            {
                temp_cy = (app_reg[reg][j] & 0x01);
                temp = ((app_flags & FLAG_C_MASK) << 7) | (app_reg[reg][j] >> 1);
                app_flags = temp_cy | (app_flags & ~FLAG_C_MASK);
                app_reg[reg][j] = temp & 0xff;
            }
        }
    }
}

/* cy -> bits -> cy */
/* cy <- bits <- cy */
void op_rox(void)
{
uint8_t param;
uint8_t nb_shifts,reg,side;
int i,j;
int temp, temp_cy;

    param = app_memory[app_pc++];
    side = param & 0x80;
    nb_shifts = (param & 0x70) >> 4;
    reg = param & 0x0f;

    if (step_mode) wprintw(wConsole,"ROX Side: %d, Shifts: %d, Reg: %d\n",side,nb_shifts,reg);

    if (side == 0)
    {
        /* left */
        for (i=0;i<nb_shifts;i++)
        {
            for (j=0;j<app_size;j++)
            {
                temp = (app_reg[reg][j] << 1) | (app_flags & FLAG_C_MASK);
                app_flags = ((temp & 0x100) >> 8) | (app_flags & ~FLAG_C_MASK);
                app_reg[reg][j] = temp & 0xff;
            }
        }
    }
    else
    {
        /* right */
        for (i=0;i<nb_shifts;i++)
        {
            for (j=0;j<app_size;j++)
            {
                temp_cy = (app_reg[reg][j] & 0x01);
                temp = ((app_flags & FLAG_C_MASK) << 7) | (app_reg[reg][j] >> 1);
                app_flags = temp_cy | (app_flags & ~FLAG_C_MASK);
                app_reg[reg][j] = temp & 0xff;
            }
        }
    }
}

/* 0 -> bits -> cy */
/* cy <- bits <- 0 */
void op_lsx(void)
{
uint8_t param;
uint8_t nb_shifts,reg,side;
int i,j;
int temp, temp_cy;

    param = app_memory[app_pc++];
    side = param & 0x80;
    nb_shifts = (param & 0x70) >> 4;
    reg = param & 0x0f;

    if (step_mode) wprintw(wConsole,"LSX Side: %d, Shifts: %d, Reg: %d\n",side,nb_shifts,reg);

    if (side == 0)
    {
        /* left */
        for (i=0;i<nb_shifts;i++)
        {
            /* Clear carry flag */
            app_flags &= ~FLAG_C_MASK;

            for (j=0;j<app_size;j++)
            {
                temp = (app_reg[reg][j] << 1) | (app_flags & FLAG_C_MASK);
                app_flags = ((temp & 0x100) >> 8) | (app_flags & ~FLAG_C_MASK);
                app_reg[reg][j] = temp & 0xff;
            }
        }
    }
    else
    {
        /* right */
        for (i=0;i<nb_shifts;i++)
        {
            /* Clear carry flag */
            app_flags &= ~FLAG_C_MASK;

            for (j=0;j<app_size;j++)
            {
                temp_cy = (app_reg[reg][j] & 0x01);
                temp = ((app_flags & FLAG_C_MASK) << 7) | (app_reg[reg][j] >> 1);
                app_flags = temp_cy | (app_flags & ~FLAG_C_MASK);
                app_reg[reg][j] = temp & 0xff;
            }
        }
    }
}

