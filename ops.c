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
                temp = reverse_byte(app_reg[reg].B[(app_size-1)-i]);
                app_reg[reg].B[(app_size-1)-i] = reverse_byte(app_reg[reg].B[i]);
                app_reg[reg].B[i] = temp;
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
                app_reg[reg].B[i] = sxt_byte();
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
                app_reg[reg].B[i] = ~app_reg[reg].B[i];
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
                app_reg[reg].B[i] = ~app_reg[reg].B[i];
            }

            app_flags |= FLAG_C_MASK;
            for (i=0;i<app_size;i++)
            {
                app_reg[reg].B[i] = byte_adc(app_reg[reg].B[i],0);
            }

            break;
        default:
            illegal_inst();
    }
}

int16_t adrModeRelativeTableIndex(void)
{
	int16_t disp;
	int16_t addr;
	int16_t offset;

    getPair();
    disp = getword(app_pc);
    offset = get_addr(src)*app_size;
    addr = app_pc-3 -size_byte + disp + offset;

    if (step_mode) wprintw(wConsole,"Table relative: base: %04X, disp: %04X, offset:%04X, addr:%04X\n",
        app_pc-3 -size_byte,
        disp,
        offset,
        addr);

    app_pc +=2;

    return addr;
}

void op_st(void)
{
uint8_t reg;
int16_t addr;
int i;

    reg = adr_mode & 0x0f;

    switch(adr_mode & 0xf0)
    {
        case 0xE0: //r,(addr)
            addr = getword(app_pc);
            app_pc +=2;

            for (i=0;i<app_size;i++)
            {
                app_memory[addr++] = app_reg[reg].B[i];
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
            if (extended)
            {
                getRegIndex();

                switch(app_size)
                {
                    case 1:
                        i1 = temp1 & 0x0f;
                        i2 = temp2 & 0x0f;
                        app_reg[dest].B[i1] = app_reg[src].B[i2];
                        break;
                    case 2:
                        i1 = temp1 & 0x07;
                        i2 = temp2 & 0x07;
                        app_reg[dest].W[i1] = app_reg[src].W[i2];
                        break;
                    case 4:
                        i1 = temp1 & 0x03;
                        i2 = temp2 & 0x03;
                        app_reg[dest].D[i1] = app_reg[src].D[i2];
                        break;
                    case 8:
                        i1 = temp1 & 0x01;
                        i2 = temp2 & 0x01;
                        app_reg[dest].Q[i1] = app_reg[src].Q[i2];
                        break;
                    default:
                        illegal_inst();
                }
            }
            else
            {
                getPair();

                for (i=0;i<app_size;i++)
                {
                    app_reg[dest].B[i] = app_reg[src].B[i];
                }
            }
            break;

        case 0x30:
            switch (adr_mode)
            {
                case 0x35: //R,(~ADDR)[R]
                    addr = adrModeRelativeTableIndex();

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest].B[i] = app_memory[addr++];
                    }
                    break;

                case 0x36: //R,(ADDR)[R]
                    getPair();
                    addr = getword(app_pc) + get_addr(src)*app_size;
                    app_pc +=2;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest].B[i] = app_memory[addr++];
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
                        app_reg[dest].B[i] = app_memory[addr++];
                    }
                    break;

                case 0x39: //r,*p++
                    getPair();
                    addr = get_addr(src);

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest].B[i] = app_memory[addr++];
                    }

                    set_addr(src,addr);
                    break;

                case 0x3A: //r,--*p
                    {
                        uint16_t temp;

                        getPair();
                        temp = get_addr(src) - app_size;
                        addr = temp;

                        for (i=0;i<app_size;i++)
                        {
                            app_reg[dest].B[i] = app_memory[addr++];
                        }
                        set_addr(src,temp);
                    }
                    break;

                case 0x3B: //r,p[d] d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];
                    addr = get_addr(src) + disp * app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest].B[i] = app_memory[addr++];
                    }
                    break;

                case 0x3C: //*p,r
                    getPair();
                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] = app_reg[src].B[i];
                    }
                    break;

                case 0x3D: //*p++,r
                    getPair();
                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] = app_reg[src].B[i];
                    }
                    set_addr(dest,addr);
                    break;

                case 0x3E: //--*p,r
                    {
                    uint16_t temp;

                        getPair();
                        temp = get_addr(dest) - app_size;
                        addr = temp;

                        for (i=0;i<app_size;i++)
                        {
                             app_memory[addr++] = app_reg[src].B[i];
                        }

                        set_addr(dest,temp);
                    }
                    break;

                case 0x3F: //p[d],r d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];

                    addr = get_addr(dest) + disp * app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] = app_reg[src].B[i];
                    }
                    break;

                default:
                    illegal_inst();
            }
            break;

        case 0xE0: //r,(addr)
            addr = getword(app_pc);
            app_pc +=2;

            for (i=0;i<app_size;i++)
            {
                app_reg[reg].B[i] = app_memory[addr++];
            }
            break;

        case 0xF0: //r,#
            for (i=0;i<app_size;i++)
            {
                app_reg[reg].B[i] = app_memory[app_pc++];
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
        app_memory[addr++] = app_reg[reg].B[i];
        app_reg[reg].B[i] = temp;
    }
    return addr;
}

void op_ex(void)
{
uint8_t reg,disp,temp;
int16_t addr;
int i;

    reg = adr_mode & 0x0f;

    switch(adr_mode & 0xf0)
    {
        case 0:
            if (extended)
            {
                getRegIndex();

                switch(app_size)
                {
                    case 1:
                        i1 = temp1 & 0x0f;
                        i2 = temp2 & 0x0f;
                        temp = app_reg[src].B[i2];
                        app_reg[src].B[i2] = app_reg[dest].B[i1];
                        app_reg[dest].B[i1] = temp;
                        break;
                    case 2:
                        i1 = temp1 & 0x07;
                        i2 = temp2 & 0x07;
                        temp = app_reg[src].W[i2];
                        app_reg[src].W[i2] = app_reg[dest].W[i1];
                        app_reg[dest].W[i1] = temp;
                        break;
                    case 4:
                        i1 = temp1 & 0x03;
                        i2 = temp2 & 0x03;
                        temp = app_reg[src].D[i2];
                        app_reg[src].D[i2] = app_reg[dest].D[i1];
                        app_reg[dest].D[i1] = temp;
                        break;
                    case 8:
                        i1 = temp1 & 0x01;
                        i2 = temp2 & 0x01;
                        temp = app_reg[src].Q[i2];
                        app_reg[src].Q[i2] = app_reg[dest].Q[i1];
                        app_reg[dest].Q[i1] = temp;
                        break;
                    default:
                        illegal_inst();
                }
            }
            else
            {
                getPair();

                for (i=0;i<app_size;i++)
                {
                    temp = app_reg[src].B[i];
                    app_reg[src].B[i] = app_reg[dest].B[i];
                    app_reg[dest].B[i] = temp;
                }
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
                    {
                        uint16_t temp;

                        getPair();

                        temp = get_addr(src) - app_size;
                        addr = temp;
                        addr = ex_reg_memory(addr,dest);
                        set_addr(src,temp);
                    }
                    break;

                case 0x3e: //--*p,r
                    {
                        uint16_t temp;

                        getPair();

                        temp = get_addr(dest) - app_size;
                        addr = temp;
                        addr = ex_reg_memory(addr,src);
                        set_addr(dest,temp);
                    }
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

        case 0xE0: //r,(addr)
            addr = getword(app_pc);
            app_pc +=2;
            ex_reg_memory(addr,reg);
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
            if (extended)
            {
                getRegIndex();

                switch(app_size)
                {
                    case 1:
                        i1 = temp1 & 0x0f;
                        i2 = temp2 & 0x0f;
                        app_reg[dest].B[i1] = byte_add(app_reg[dest].B[i1],app_reg[src].B[i2]);
                        break;
                    case 2:
                        i1 = temp1 & 0x07;
                        i2 = temp2 & 0x07;
                        app_reg[dest].W[i1] = byte_add(app_reg[dest].W[i1],app_reg[src].W[i2]);
                        break;
                    case 4:
                        i1 = temp1 & 0x03;
                        i2 = temp2 & 0x03;
                        app_reg[dest].D[i1] = byte_add(app_reg[dest].D[i1],app_reg[src].D[i2]);
                        break;
                    case 8:
                        i1 = temp1 & 0x01;
                        i2 = temp2 & 0x01;
                        app_reg[dest].Q[i1] = byte_add(app_reg[dest].Q[i1],app_reg[src].Q[i2]);
                        break;
                    default:
                        illegal_inst();
                }
            }
            else
            {
                getPair();

                app_reg[dest].B[0] = byte_add(app_reg[dest].B[0],app_reg[src].B[0]);

                for (i=1;i<app_size;i++)
                {
                    app_reg[dest].B[i] = byte_adc(app_reg[dest].B[i],app_reg[src].B[i]);
                }
                setflags(&app_reg[dest].B[0]);
            }
            break;
        case 0x30:
            switch (adr_mode)
            {
                case 0x35: //R,(~ADDR)[R]
                    addr = adrModeRelativeTableIndex();

                    app_reg[dest].B[0] = byte_add(app_reg[dest].B[0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        app_reg[dest].B[i] = byte_adc(app_reg[dest].B[i],app_memory[addr++]);
                    }
                    break;

                case 0x36: //R,(ADDR)[R]
                    getPair();
                    addr = getword(app_pc) + get_addr(src)*app_size;
                    app_pc +=2;

                    app_reg[dest].B[0] = byte_add(app_reg[dest].B[0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        app_reg[dest].B[i] = byte_adc(app_reg[dest].B[i],app_memory[addr++]);
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

                    app_reg[dest].B[0] = byte_add(app_reg[dest].B[0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        app_reg[dest].B[i] = byte_adc(app_reg[dest].B[i],app_memory[addr++]);
                    }
                    break;

                case 0x39: //r,*p++
                    getPair();
                    addr = get_addr(src);

                    app_reg[dest].B[0] = byte_add(app_reg[dest].B[0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        app_reg[dest].B[i] = byte_adc(app_reg[dest].B[i],app_memory[addr++]);
                    }

                    set_addr(src,addr);
                    break;

                case 0x3A: //r,--*p
                    {
                    uint16_t temp;

                        getPair();
                        temp = get_addr(src) - app_size;
                        addr = temp;

                        app_reg[dest].B[0] = byte_add(app_reg[dest].B[0],app_memory[addr++]);

                        for (i=1;i<app_size;i++)
                        {
                            app_reg[dest].B[i] = byte_adc(app_reg[dest].B[i],app_memory[addr++]);
                        }
                        set_addr(src,temp);
                    }
                    break;

                case 0x3B: //r,p[d] d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];
                    addr = get_addr(src) + disp * app_size;

                    app_reg[dest].B[0] = byte_add(app_reg[dest].B[0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        app_reg[dest].B[i] = byte_adc(app_reg[dest].B[i],app_memory[addr++]);
                    }
                    break;

                case 0x3C: //*p,r
                    getPair();
                    addr = get_addr(dest);

                    app_memory[addr] = byte_add(app_memory[addr],app_reg[src].B[0]);
                    addr++;

                    for (i=1;i<app_size;i++)
                    {
                        app_memory[addr] = byte_adc(app_memory[addr],app_reg[src].B[i]);
                        addr++;
                    }
                    break;

                case 0x3D: //*p++,r
                    getPair();
                    addr = get_addr(dest);

                    app_memory[addr] = byte_add(app_memory[addr],app_reg[src].B[0]);
                    addr++;

                    for (i=1;i<app_size;i++)
                    {
                        app_memory[addr] = byte_adc(app_memory[addr],app_reg[src].B[i]);
                        addr++;
                    }
                    set_addr(dest,addr);
                    break;

                case 0x3E: //--*p,r
                    {
                    uint16_t temp;

                        getPair();
                        temp = get_addr(dest) - app_size;
                        addr = temp;

                        app_memory[addr] = byte_add(app_memory[addr],app_reg[src].B[0]);
                        addr++;

                        for (i=1;i<app_size;i++)
                        {
                            app_memory[addr] = byte_adc(app_memory[addr],app_reg[src].B[i]);
                            addr++;
                        }

                        set_addr(dest,temp);
                    }
                    break;

                case 0x3F: //p[d],r d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];
                    addr = get_addr(dest) + disp * app_size;

                    app_memory[addr] = byte_add(app_memory[addr],app_reg[src].B[0]);
                    addr++;

                    for (i=1;i<app_size;i++)
                    {
                        app_memory[addr] = byte_adc(app_memory[addr],app_reg[src].B[i]);
                        addr++;
                    }
                    break;

                default:
                    illegal_inst();
            }
            break;

        case 0xE0: //r,(addr)
            addr = getword(app_pc);
            app_pc +=2;

            app_reg[reg].B[0] = byte_add(app_reg[reg].B[0],app_memory[addr++]);

            for (i=1;i<app_size;i++)
            {
                app_reg[reg].B[i] = byte_adc(app_reg[reg].B[i],app_memory[addr++]);
            }
            break;

        case 0xF0: //r,#
            app_reg[reg].B[0] = byte_add(app_reg[reg].B[0],app_memory[app_pc++]);

            for (i=1;i<app_size;i++)
            {
                app_reg[reg].B[i] = byte_adc(app_reg[reg].B[i],app_memory[app_pc++]);
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
            if (extended)
            {
                getRegIndex();

                switch(app_size)
                {
                    case 1:
                        i1 = temp1 & 0x0f;
                        i2 = temp2 & 0x0f;
                        app_reg[dest].B[i1] = byte_adc(app_reg[dest].B[i1],app_reg[src].B[i2]);
                        break;
                    case 2:
                        i1 = temp1 & 0x07;
                        i2 = temp2 & 0x07;
                        app_reg[dest].W[i1] = byte_adc(app_reg[dest].W[i1],app_reg[src].W[i2]);
                        break;
                    case 4:
                        i1 = temp1 & 0x03;
                        i2 = temp2 & 0x03;
                        app_reg[dest].D[i1] = byte_adc(app_reg[dest].D[i1],app_reg[src].D[i2]);
                        break;
                    case 8:
                        i1 = temp1 & 0x01;
                        i2 = temp2 & 0x01;
                        app_reg[dest].Q[i1] = byte_adc(app_reg[dest].Q[i1],app_reg[src].Q[i2]);
                        break;
                    default:
                        illegal_inst();
                }
            }
            else
            {
                getPair();

                for (i=0;i<app_size;i++)
                {
                    app_reg[dest].B[i] = byte_adc(app_reg[dest].B[i],app_reg[src].B[i]);
                }

                setflags(&app_reg[dest].B[0]);
            }
            break;
        case 0x30:
            switch (adr_mode)
            {
                case 0x35: //R,(~ADDR)[R]
                    addr = adrModeRelativeTableIndex();

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest].B[i] = byte_adc(app_reg[dest].B[i],app_memory[addr++]);
                    }
                    break;

                case 0x36: //R,(ADDR)[R]
                    getPair();
                    addr = getword(app_pc) + get_addr(src)*app_size;
                    app_pc +=2;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest].B[i] = byte_adc(app_reg[dest].B[i],app_memory[addr++]);
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
                        app_reg[dest].B[i] = byte_adc(app_reg[dest].B[i],app_memory[addr++]);
                    }
                    break;

                case 0x39: //r,*p++
                    getPair();
                    addr = get_addr(src);

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest].B[i] = byte_adc(app_reg[dest].B[i],app_memory[addr++]);
                    }

                    set_addr(src,addr);
                    break;

                case 0x3A: //r,--*p
                    {
                    uint16_t temp;

                        getPair();
                        temp = get_addr(src) - app_size;
                        addr = temp;

                        for (i=0;i<app_size;i++)
                        {
                            app_reg[dest].B[i] = byte_adc(app_reg[dest].B[i],app_memory[addr++]);
                        }
                        set_addr(src,temp);
                    }
                    break;

                case 0x3B: //r,p[d] d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];
                    addr = get_addr(src) + disp * app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest].B[i] = byte_adc(app_reg[dest].B[i],app_memory[addr++]);
                    }
                    break;

                case 0x3C: //*p,r
                    getPair();
                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr] = byte_adc(app_memory[addr],app_reg[src].B[i]);
                        addr++;
                    }
                    break;

                case 0x3D: //*p++,r
                    getPair();
                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr] = byte_adc(app_memory[addr],app_reg[src].B[i]);
                        addr++;
                    }
                    set_addr(dest,addr);
                    break;

                case 0x3E: //--*p,r
                    {
                    uint16_t temp;

                        getPair();
                        temp = get_addr(dest) - app_size;
                        addr = temp;

                        for (i=0;i<app_size;i++)
                        {
                            app_memory[addr] = byte_adc(app_memory[addr],app_reg[src].B[i]);
                            addr++;
                        }

                        set_addr(dest,temp);
                    }
                    break;

                case 0x3F: //p[d],r d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];
                    addr = get_addr(dest) + disp * app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr] = byte_adc(app_memory[addr],app_reg[src].B[i]);
                        addr++;
                    }
                    break;

                default:
                    illegal_inst();
            }
            break;

        case 0xE0: //r,(addr)
            addr = getword(app_pc);
            app_pc +=2;

            for (i=0;i<app_size;i++)
            {
                app_reg[reg].B[i] = byte_adc(app_reg[reg].B[i],app_memory[addr++]);
            }
            break;

        case 0xF0: //r,#
            for (i=0;i<app_size;i++)
            {
                app_reg[reg].B[i] = byte_adc(app_reg[reg].B[i],app_memory[app_pc++]);
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
            if (extended)
            {
                getRegIndex();

                switch(app_size)
                {
                    case 1:
                        i1 = temp1 & 0x0f;
                        i2 = temp2 & 0x0f;
                        app_reg[dest].B[i1] = byte_sub(app_reg[dest].B[i1],app_reg[src].B[i2]);
                        break;
                    case 2:
                        i1 = temp1 & 0x07;
                        i2 = temp2 & 0x07;
                        app_reg[dest].W[i1] = byte_sub(app_reg[dest].W[i1],app_reg[src].W[i2]);
                        break;
                    case 4:
                        i1 = temp1 & 0x03;
                        i2 = temp2 & 0x03;
                        app_reg[dest].D[i1] = byte_sub(app_reg[dest].D[i1],app_reg[src].D[i2]);
                        break;
                    case 8:
                        i1 = temp1 & 0x01;
                        i2 = temp2 & 0x01;
                        app_reg[dest].Q[i1] = byte_sub(app_reg[dest].Q[i1],app_reg[src].Q[i2]);
                        break;
                    default:
                        illegal_inst();
                }
            }
            else
            {
                getPair();

                app_reg[dest].B[0] = byte_sub(app_reg[dest].B[0],app_reg[src].B[0]);

                for (i=1;i<app_size;i++)
                {
                    app_reg[dest].B[i] = byte_sbc(app_reg[dest].B[i],app_reg[src].B[i]);
                }
                setflags(&app_reg[dest].B[0]);
            }
            break;
        case 0x30:
            switch (adr_mode)
            {
                case 0x35: //R,(~ADDR)[R]
                    addr = adrModeRelativeTableIndex();

                    app_reg[dest].B[0] = byte_sub(app_reg[dest].B[0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        app_reg[dest].B[i] = byte_sbc(app_reg[dest].B[i],app_memory[addr++]);
                    }
                    break;

                case 0x36: //R,(ADDR)[R]
                    getPair();
                    addr = getword(app_pc) + get_addr(src)*app_size;
                    app_pc +=2;

                    app_reg[dest].B[0] = byte_sub(app_reg[dest].B[0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        app_reg[dest].B[i] = byte_sbc(app_reg[dest].B[i],app_memory[addr++]);
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

                    app_reg[dest].B[0] = byte_sub(app_reg[dest].B[0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        app_reg[dest].B[i] = byte_sbc(app_reg[dest].B[i],app_memory[addr++]);
                    }
                    break;

                case 0x39: //r,*p++
                    getPair();
                    addr = get_addr(src);

                    app_reg[dest].B[0] = byte_sub(app_reg[dest].B[0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        app_reg[dest].B[i] = byte_sbc(app_reg[dest].B[i],app_memory[addr++]);
                    }

                    set_addr(src,addr);
                    break;

                case 0x3A: //r,--*p
                    {
                    uint16_t temp;

                        getPair();
                        temp = get_addr(src) - app_size;
                        addr = temp;

                        app_reg[dest].B[0] = byte_sub(app_reg[dest].B[0],app_memory[addr++]);

                        for (i=1;i<app_size;i++)
                        {
                            app_reg[dest].B[i] = byte_sbc(app_reg[dest].B[i],app_memory[addr++]);
                        }
                        set_addr(src,temp);
                    }
                    break;

                case 0x3B: //r,p[d] d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];
                    addr = get_addr(src) + disp * app_size;

                    app_reg[dest].B[0] = byte_sub(app_reg[dest].B[0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        app_reg[dest].B[i] = byte_sbc(app_reg[dest].B[i],app_memory[addr++]);
                    }
                    break;

                case 0x3C: //*p,r
                    getPair();
                    addr = get_addr(dest);

                    app_memory[addr] = byte_sub(app_memory[addr],app_reg[src].B[0]);
                    addr++;

                    for (i=1;i<app_size;i++)
                    {
                        app_memory[addr] = byte_sbc(app_memory[addr],app_reg[src].B[i]);
                        addr++;
                    }
                    break;

                case 0x3D: //*p++,r
                    getPair();
                    addr = get_addr(dest);

                    app_memory[addr] = byte_sub(app_memory[addr],app_reg[src].B[0]);
                    addr++;

                    for (i=1;i<app_size;i++)
                    {
                        app_memory[addr] = byte_sbc(app_memory[addr],app_reg[src].B[i]);
                        addr++;
                    }
                    set_addr(dest,addr);
                    break;

                case 0x3E: //--*p,r
                    {
                    uint16_t temp;

                        getPair();
                        temp = get_addr(dest) - app_size;
                        addr = temp;

                        app_memory[addr] = byte_sub(app_memory[addr],app_reg[src].B[0]);
                        addr++;

                        for (i=1;i<app_size;i++)
                        {
                            app_memory[addr] = byte_sbc(app_memory[addr],app_reg[src].B[i]);
                            addr++;
                        }

                        set_addr(dest,temp);
                    }
                    break;

                case 0x3F: //p[d],r d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];
                    addr = get_addr(dest) + disp * app_size;

                    app_memory[addr] = byte_sub(app_memory[addr],app_reg[src].B[0]);
                    addr++;

                    for (i=1;i<app_size;i++)
                    {
                        app_memory[addr] = byte_sbc(app_memory[addr],app_reg[src].B[i]);
                        addr++;
                    }
                    break;

                default:
                    illegal_inst();
            }
            break;

        case 0xE0: //r,(addr)
            addr = getword(app_pc);
            app_pc +=2;

            app_reg[reg].B[0] = byte_sub(app_reg[reg].B[0],app_memory[addr++]);

            for (i=1;i<app_size;i++)
            {
                app_reg[reg].B[i] = byte_sbc(app_reg[reg].B[i],app_memory[addr++]);
            }
            break;

        case 0xF0: //r,#
            app_reg[reg].B[0] = byte_sub(app_reg[reg].B[0],app_memory[app_pc++]);

            for (i=1;i<app_size;i++)
            {
                app_reg[reg].B[i] = byte_sbc(app_reg[reg].B[i],app_memory[app_pc++]);
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
            if (extended)
            {
                getRegIndex();

                switch(app_size)
                {
                    case 1:
                        i1 = temp1 & 0x0f;
                        i2 = temp2 & 0x0f;
                        app_reg[dest].B[i1] = byte_sbc(app_reg[dest].B[i1],app_reg[src].B[i2]);
                        break;
                    case 2:
                        i1 = temp1 & 0x07;
                        i2 = temp2 & 0x07;
                        app_reg[dest].W[i1] = byte_sbc(app_reg[dest].W[i1],app_reg[src].W[i2]);
                        break;
                    case 4:
                        i1 = temp1 & 0x03;
                        i2 = temp2 & 0x03;
                        app_reg[dest].D[i1] = byte_sbc(app_reg[dest].D[i1],app_reg[src].D[i2]);
                        break;
                    case 8:
                        i1 = temp1 & 0x01;
                        i2 = temp2 & 0x01;
                        app_reg[dest].Q[i1] = byte_sbc(app_reg[dest].Q[i1],app_reg[src].Q[i2]);
                        break;
                    default:
                        illegal_inst();
                }
            }
            else
            {
                getPair();

                for (i=0;i<app_size;i++)
                {
                    app_reg[dest].B[i] = byte_sbc(app_reg[dest].B[i],app_reg[src].B[i]);
                }
                setflags(&app_reg[dest].B[0]);
            }
            break;
        case 0x30:
            switch (adr_mode)
            {
                case 0x35: //R,(~ADDR)[R]
                    addr = adrModeRelativeTableIndex();

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest].B[i] = byte_sbc(app_reg[dest].B[i],app_memory[addr++]);
                    }
                    break;

                case 0x36: //R,(ADDR)[R]
                    getPair();
                    addr = getword(app_pc) + get_addr(src)*app_size;
                    app_pc +=2;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest].B[i] = byte_sbc(app_reg[dest].B[i],app_memory[addr++]);
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
                        app_reg[dest].B[i] = byte_sbc(app_reg[dest].B[i],app_memory[addr++]);
                    }
                    break;

                case 0x39: //r,*p++
                    getPair();
                    addr = get_addr(src);

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest].B[i] = byte_sbc(app_reg[dest].B[i],app_memory[addr++]);
                    }

                    set_addr(src,addr);
                    break;

                case 0x3A: //r,--*p
                    {
                        uint16_t temp;

                        getPair();
                        temp = get_addr(src) - app_size;
                        addr = temp;

                        for (i=0;i<app_size;i++)
                        {
                            app_reg[dest].B[i] = byte_sbc(app_reg[dest].B[i],app_memory[addr++]);
                        }
                        set_addr(src,temp);
                    }
                    break;

                case 0x3B: //r,p[d] d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];
                    addr = get_addr(src) + disp * app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest].B[i] = byte_sbc(app_reg[dest].B[i],app_memory[addr++]);
                    }
                    break;

                case 0x3C: //*p,r
                    getPair();
                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr] = byte_sbc(app_memory[addr],app_reg[src].B[i]);
                        addr++;
                    }
                    break;

                case 0x3D: //*p++,r
                    getPair();
                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr] = byte_sbc(app_memory[addr],app_reg[src].B[i]);
                        addr++;
                    }
                    set_addr(dest,addr);
                    break;

                case 0x3E: //--*p,r
                    {
                    uint16_t temp;

                        getPair();
                        temp = get_addr(dest) - app_size;
                        addr = temp;

                        for (i=0;i<app_size;i++)
                        {
                            app_memory[addr] = byte_sbc(app_memory[addr],app_reg[src].B[i]);
                            addr++;
                        }

                        set_addr(dest,temp);
                    }
                    break;

                case 0x3F: //p[d],r d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];
                    addr = get_addr(dest) + disp * app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr] = byte_sbc(app_memory[addr],app_reg[src].B[i]);
                        addr++;
                    }
                    break;

                default:
                    illegal_inst();
            }
            break;

        case 0xE0: //r,(addr)
            addr = getword(app_pc);
            app_pc +=2;

            for (i=0;i<app_size;i++)
            {
                app_reg[reg].B[i] = byte_sbc(app_reg[reg].B[i],app_memory[addr++]);
            }
            break;

        case 0xF0: //r,#
            for (i=0;i<app_size;i++)
            {
                app_reg[reg].B[i] = byte_sbc(app_reg[reg].B[i],app_memory[app_pc++]);
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
            if (extended)
            {
                getRegIndex();

                switch(app_size)
                {
                    case 1:
                        i1 = temp1 & 0x0f;
                        i2 = temp2 & 0x0f;
                        byte_sub(app_reg[dest].B[i1],app_reg[src].B[i2]);
                        break;
                    case 2:
                        i1 = temp1 & 0x07;
                        i2 = temp2 & 0x07;
                        byte_sub(app_reg[dest].W[i1],app_reg[src].W[i2]);
                        break;
                    case 4:
                        i1 = temp1 & 0x03;
                        i2 = temp2 & 0x03;
                        byte_sub(app_reg[dest].D[i1],app_reg[src].D[i2]);
                        break;
                    case 8:
                        i1 = temp1 & 0x01;
                        i2 = temp2 & 0x01;
                        byte_sub(app_reg[dest].Q[i1],app_reg[src].Q[i2]);
                        break;
                    default:
                        illegal_inst();
                }
            }
            else
            {
                getPair();

                byte_sub(app_reg[dest].B[0],app_reg[src].B[0]);

                for (i=1;i<app_size;i++)
                {
                    byte_sbc(app_reg[dest].B[i],app_reg[src].B[i]);
                }
            }
            break;
        case 0x30:
            switch (adr_mode)
            {
                case 0x35: //R,(~ADDR)[R]
                    addr = adrModeRelativeTableIndex();

                    byte_sub(app_reg[dest].B[0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        byte_sbc(app_reg[dest].B[i],app_memory[addr++]);
                    }
                    break;

                case 0x36: //R,(ADDR)[R]
                    getPair();
                    addr = getword(app_pc) + get_addr(src)*app_size;
                    app_pc +=2;

                    byte_sub(app_reg[dest].B[0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        byte_sbc(app_reg[dest].B[i],app_memory[addr++]);
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

                    byte_sub(app_reg[dest].B[0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        byte_sbc(app_reg[dest].B[i],app_memory[addr++]);
                    }
                    break;

                case 0x39: //r,*p++
                    getPair();
                    addr = get_addr(src);

                    byte_sub(app_reg[dest].B[0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        byte_sbc(app_reg[dest].B[i],app_memory[addr++]);
                    }

                    set_addr(src,addr);
                    break;

                case 0x3A: //r,--*p
                    {
                    uint16_t temp;

                        getPair();
                        temp = get_addr(src) - app_size;
                        addr = temp;

                        byte_sub(app_reg[dest].B[0],app_memory[addr++]);

                        for (i=1;i<app_size;i++)
                        {
                            byte_sbc(app_reg[dest].B[i],app_memory[addr++]);
                        }
                        set_addr(src,temp);
                    }
                    break;

                case 0x3B: //r,p[d] d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];
                    addr = get_addr(src) + disp * app_size;

                    byte_sub(app_reg[dest].B[0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        byte_sbc(app_reg[dest].B[i],app_memory[addr++]);
                    }
                    break;

                case 0x3C: //*p,r
                    getPair();
                    addr = get_addr(dest);

                    byte_sub(app_memory[addr++],app_reg[src].B[0]);

                    for (i=1;i<app_size;i++)
                    {
                        byte_sbc(app_memory[addr++],app_reg[src].B[i]);
                    }
                    break;

                case 0x3D: //*p++,r
                    getPair();
                    addr = get_addr(dest);

                    byte_sub(app_memory[addr++],app_reg[src].B[0]);

                    for (i=1;i<app_size;i++)
                    {
                        byte_sbc(app_memory[addr++],app_reg[src].B[i]);
                    }
                    set_addr(dest,addr);
                    break;

                case 0x3E: //--*p,r
                    {
                    uint16_t temp;

                        getPair();
                        temp = get_addr(dest) - app_size;
                        addr = temp;

                        byte_sub(app_memory[addr++],app_reg[src].B[0]);

                        for (i=1;i<app_size;i++)
                        {
                            byte_sbc(app_memory[addr++],app_reg[src].B[i]);
                        }

                        set_addr(dest,temp);
                    }
                    break;

                case 0x3F: //p[d],r d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];
                    addr = get_addr(dest) + disp * app_size;

                    byte_sub(app_memory[addr++],app_reg[src].B[0]);

                    for (i=1;i<app_size;i++)
                    {
                        byte_sbc(app_memory[addr++],app_reg[src].B[i]);
                    }
                    break;

                default:
                    illegal_inst();
            }
            break;

        case 0xE0: //r,(addr)
            addr = getword(app_pc);
            app_pc +=2;

            byte_sub(app_reg[reg].B[0],app_memory[addr++]);

            for (i=1;i<app_size;i++)
            {
                byte_sbc(app_reg[reg].B[i],app_memory[addr++]);
            }
            break;

        case 0xF0: //r,#
            byte_sub(app_reg[reg].B[0],app_memory[app_pc++]);

            for (i=1;i<app_size;i++)
            {
                byte_sbc(app_reg[reg].B[i],app_memory[app_pc++]);
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
            if (extended)
            {
                getRegIndex();

                switch(app_size)
                {
                    case 1:
                        i1 = temp1 & 0x0f;
                        i2 = temp2 & 0x0f;
                        app_reg[dest].B[i1] &= app_reg[src].B[i2];
                        break;
                    case 2:
                        i1 = temp1 & 0x07;
                        i2 = temp2 & 0x07;
                        app_reg[dest].W[i1] &= app_reg[src].W[i2];
                        break;
                    case 4:
                        i1 = temp1 & 0x03;
                        i2 = temp2 & 0x03;
                        app_reg[dest].D[i1] &= app_reg[src].D[i2];
                        break;
                    case 8:
                        i1 = temp1 & 0x01;
                        i2 = temp2 & 0x01;
                        app_reg[dest].Q[i1] &= app_reg[src].Q[i2];
                        break;
                    default:
                        illegal_inst();
                }
            }
            else
            {
                getPair();

                for (i=0;i<app_size;i++)
                {
                    app_reg[dest].B[i] &= app_reg[src].B[i];
                }
                setflags(&app_reg[dest].B[0]);
            }
            break;
        case 0x30:
            switch (adr_mode)
            {
                case 0x35: //R,(~ADDR)[R]
                    addr = adrModeRelativeTableIndex();

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest].B[i] &= app_memory[addr++];
                    }
                    break;

                case 0x36: //R,(ADDR)[R]
                    getPair();
                    addr = getword(app_pc) + get_addr(src)*app_size;
                    app_pc +=2;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest].B[i] &= app_memory[addr++];
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
                        app_reg[dest].B[i] &= app_memory[addr++];
                    }
                    break;

                case 0x39: //r,*p++
                    getPair();
                    addr = get_addr(src);

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest].B[i] &= app_memory[addr++];
                    }

                    set_addr(src,addr);
                    break;

                case 0x3A: //r,--*p
                    {
                    uint16_t temp;

                        getPair();
                        temp = get_addr(src) - app_size;
                        addr = temp;

                        for (i=0;i<app_size;i++)
                        {
                            app_reg[dest].B[i] &= app_memory[addr++];
                        }
                        set_addr(src,temp);
                    }
                    break;

                case 0x3B: //r,p[d] d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];
                    addr = get_addr(src) + disp * app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest].B[i] &= app_memory[addr++];
                    }
                    break;

                case 0x3C: //*p,r
                    getPair();
                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] &= app_reg[src].B[i];
                    }
                    break;

                case 0x3D: //*p++,r
                    getPair();
                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] &= app_reg[src].B[i];
                    }
                    set_addr(dest,addr);
                    break;

                case 0x3E: //--*p,r
                    {
                    uint16_t temp;

                        getPair();
                        temp = get_addr(dest) - app_size;
                        addr = temp;

                        for (i=0;i<app_size;i++)
                        {
                             app_memory[addr++] &= app_reg[src].B[i];
                        }

                        set_addr(dest,temp);
                    }
                    break;

                case 0x3F: //p[d],r d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];
                    addr = get_addr(dest) + disp * app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] &= app_reg[src].B[i];
                    }
                    break;

                default:
                    illegal_inst();
            }
            break;

        case 0xE0: //r,(addr)
            addr = getword(app_pc);
            app_pc +=2;

            for (i=1;i<app_size;i++)
            {
                app_reg[reg].B[i] &= app_memory[addr++];
            }
            break;

        case 0xF0: //r,#
            for (i=0;i<app_size;i++)
            {
                app_reg[reg].B[i] &= app_memory[app_pc++];
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
            if (extended)
            {
                getRegIndex();

                switch(app_size)
                {
                    case 1:
                        i1 = temp1 & 0x0f;
                        i2 = temp2 & 0x0f;
                        app_reg[dest].B[i1] |= app_reg[src].B[i2];
                        break;
                    case 2:
                        i1 = temp1 & 0x07;
                        i2 = temp2 & 0x07;
                        app_reg[dest].W[i1] |= app_reg[src].W[i2];
                        break;
                    case 4:
                        i1 = temp1 & 0x03;
                        i2 = temp2 & 0x03;
                        app_reg[dest].D[i1] |= app_reg[src].D[i2];
                        break;
                    case 8:
                        i1 = temp1 & 0x01;
                        i2 = temp2 & 0x01;
                        app_reg[dest].Q[i1] |= app_reg[src].Q[i2];
                        break;
                    default:
                        illegal_inst();
                }
            }
            else
            {
                getPair();

                for (i=0;i<app_size;i++)
                {
                    app_reg[dest].B[i] |= app_reg[src].B[i];
                }
                setflags(&app_reg[dest].B[0]);
            }
            break;
        case 0x30:
            switch (adr_mode)
            {
                case 0x35: //R,(~ADDR)[R]
                    addr = adrModeRelativeTableIndex();

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest].B[i] |= app_memory[addr++];
                    }
                    break;

                case 0x36: //R,(ADDR)[R]
                    getPair();
                    addr = getword(app_pc) + get_addr(src)*app_size;
                    app_pc +=2;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest].B[i] |= app_memory[addr++];
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
                        app_reg[dest].B[i] |= app_memory[addr++];
                    }
                    break;

                case 0x39: //r,*p++
                    getPair();
                    addr = get_addr(src);

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest].B[i] |= app_memory[addr++];
                    }

                    set_addr(src,addr);
                    break;

                case 0x3A: //r,--*p
                    {
                    uint16_t temp;

                        getPair();
                        temp = get_addr(src) - app_size;
                        addr = temp;

                        for (i=0;i<app_size;i++)
                        {
                            app_reg[dest].B[i] |= app_memory[addr++];
                        }

                        set_addr(src,temp);
                    }
                    break;

                case 0x3B: //r,p[d] d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];
                    addr = get_addr(src) + disp * app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest].B[i] |= app_memory[addr++];
                    }
                    break;

                case 0x3C: //*p,r
                    getPair();
                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] |= app_reg[src].B[i];
                    }
                    break;

                case 0x3D: //*p++,r
                    getPair();
                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] |= app_reg[src].B[i];
                    }
                    set_addr(dest,addr);
                    break;

                case 0x3E: //--*p,r
                    {
                    uint16_t temp;

                        getPair();
                        temp = get_addr(dest) - app_size;
                        addr = temp;

                        for (i=0;i<app_size;i++)
                        {
                             app_memory[addr++] |= app_reg[src].B[i];
                        }

                        set_addr(dest,temp);
                    }
                    break;

                case 0x3F: //p[d],r d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];
                    addr = get_addr(dest) + disp * app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] |= app_reg[src].B[i];
                    }
                    break;

                default:
                    illegal_inst();
            }
            break;

        case 0xE0: //r,(addr)
            addr = getword(app_pc);
            app_pc +=2;

            for (i=1;i<app_size;i++)
            {
                app_reg[reg].B[i] |= app_memory[addr++];
            }
            break;

        case 0xF0: //r,#
            for (i=0;i<app_size;i++)
            {
                app_reg[reg].B[i] |= app_memory[app_pc++];
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
            if (extended)
            {
                getRegIndex();

                switch(app_size)
                {
                    case 1:
                        i1 = temp1 & 0x0f;
                        i2 = temp2 & 0x0f;
                        app_reg[dest].B[i1] ^= app_reg[src].B[i2];
                        break;
                    case 2:
                        i1 = temp1 & 0x07;
                        i2 = temp2 & 0x07;
                        app_reg[dest].W[i1] ^= app_reg[src].W[i2];
                        break;
                    case 4:
                        i1 = temp1 & 0x03;
                        i2 = temp2 & 0x03;
                        app_reg[dest].D[i1] ^= app_reg[src].D[i2];
                        break;
                    case 8:
                        i1 = temp1 & 0x01;
                        i2 = temp2 & 0x01;
                        app_reg[dest].Q[i1] ^= app_reg[src].Q[i2];
                        break;
                    default:
                        illegal_inst();
                }
            }
            else
            {
                getPair();

                for (i=0;i<app_size;i++)
                {
                    app_reg[dest].B[i] ^= app_reg[src].B[i];
                }
                setflags(&app_reg[dest].B[0]);
            }
            break;
        case 0x30:
            switch (adr_mode)
            {
                case 0x35: //R,(~ADDR)[R]
                    addr = adrModeRelativeTableIndex();

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest].B[i] ^= app_memory[addr++];
                    }
                    break;

                case 0x36: //R,(ADDR)[R]
                    getPair();
                    addr = getword(app_pc) + get_addr(src)*app_size;
                    app_pc +=2;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest].B[i] ^= app_memory[addr++];
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
                        app_reg[dest].B[i] ^= app_memory[addr++];
                    }
                    break;

                case 0x39: //r,*p++
                    getPair();
                    addr = get_addr(src);

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest].B[i] ^= app_memory[addr++];
                    }

                    set_addr(src,addr);
                    break;

                case 0x3A: //r,--*p
                    {
                    uint16_t temp;

                        getPair();
                        temp = get_addr(src) - app_size;
                        addr = temp;

                        for (i=0;i<app_size;i++)
                        {
                            app_reg[dest].B[i] ^= app_memory[addr++];
                        }
                        set_addr(src,temp);
                    }
                    break;

                case 0x3B: //r,p[d] d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];
                    addr = get_addr(src) + disp * app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest].B[i] ^= app_memory[addr++];
                    }
                    break;

                case 0x3C: //*p,r
                    getPair();
                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] ^= app_reg[src].B[i];
                    }
                    break;

                case 0x3D: //*p++,r
                    getPair();
                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] ^= app_reg[src].B[i];
                    }
                    set_addr(dest,addr);
                    break;

                case 0x3E: //--*p,r
                    {
                    uint16_t temp;

                        getPair();
                        temp = get_addr(dest) - app_size;
                        addr = temp;

                        for (i=0;i<app_size;i++)
                        {
                             app_memory[addr++] ^= app_reg[src].B[i];
                        }

                        set_addr(dest,temp);
                    }
                    break;

                case 0x3F: //p[d],r d: index 0-n
                    getPair();
                    disp = app_memory[app_pc++];
                    addr = get_addr(dest) + disp * app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] ^= app_reg[src].B[i];
                    }
                    break;

                default:
                    illegal_inst();
            }
            break;

        case 0xE0: //r,(addr)
            addr = getword(app_pc);
            app_pc +=2;

            for (i=1;i<app_size;i++)
            {
                app_reg[reg].B[i] ^= app_memory[addr++];
            }
            break;

        case 0xF0: //r,#
            for (i=0;i<app_size;i++)
            {
                app_reg[reg].B[i] ^= app_memory[app_pc++];
            }
            break;

        default:
            illegal_inst();
    }
}

void op_msk(uint128_t *reg)
{
    switch(app_size)
    {
    case 1:
        reg->B[0] = 1 << (reg->B[0] & 0x07);
        break;
    case 2:
        reg->W[0] = 1 << (reg->B[0] & 0x0F);
        break;
    case 4:
        reg->D[0] = 1 << (reg->B[0] & 0x1F);
        break;
    case 8:
        reg->Q[0] = 1ULL << (reg->B[0] & 0x3F);
        break;
    case 16:
        {
            uint64_t temp = (1ULL << (reg->B[0] & 0x3F));

            if (reg->B[0] & 0x40)
            {
                reg->Q[0] = 0;
                reg->Q[1] = temp;
            }
            else
            {
                reg->Q[0] = temp;
                reg->Q[1] = 0;
            }
        }
        break;
    }
}

void op_swap(uint128_t *reg)
{
    switch(app_size)
    {
    case 1:
        reg->B[0] = swap_byte(reg->B[0]);
        break;
    case 2:
        {
            uint8_t temp = reg->B[0];
            reg->B[0] = reg->B[1];
            reg->B[1] = temp;
        }
        break;
    case 4:
        {
            uint16_t temp = reg->W[0];
            reg->W[0] = reg->W[1];
            reg->W[1] = temp;
        }
        break;
    case 8:
        {
            uint32_t temp = reg->D[0];
            reg->D[0] = reg->D[1];
            reg->D[1] = temp;
        }
        break;
    case 16:
        {
            uint64_t temp = reg->Q[0];
            reg->Q[0] = reg->Q[1];
            reg->Q[1] = temp;
        }
        break;
    }
}

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
int result, val;

    if (extended)
    {
        getPair();

        if (step_mode) wprintw(wConsole,"IN: R%d,(R%d) = ", dest, src);

        result = scanw("%02X",&val);
        if (step_mode) wprintw(wConsole,"Result: %02X, Input: %02X\n",result,val & 0xff);
        app_reg[dest].B[0] = val & 0xff;
    }
    else
    {
        uint8_t port = app_memory[app_pc++];

        if (step_mode) wprintw(wConsole,"IN: (%02X) = ",port);

        if (app_size == 2)
        {
            result = scanw("%04X",&val);
            if (step_mode) wprintw(wConsole,"Result: %02X, Input: %04X\n",result,val & 0xffff);

            if ((adr_mode & 0xf0) == 0xF0)
            {
                app_reg[adr_mode & 0xf].W[0] = val & 0xffff;
            }
            else
            {
                app_reg[0].W[0] = val & 0xffff;
            }
        }
        else
        {
            result = scanw("%02X",&val);
            if (step_mode) wprintw(wConsole,"Result: %02X, Input: %02X\n",result,val & 0xff);

            if ((adr_mode & 0xf0) == 0xF0)
            {
                app_reg[adr_mode & 0xf].B[0] = val & 0xff;
            }
            else
            {
                app_reg[0].B[0] = val & 0xff;
            }
        }
    }
}

void op_out(void)
{
uint8_t val;

    if (extended)
    {
        getPair();

        if (step_mode)
        {
            val = app_reg[src].B[0];
            wprintw(wConsole,"OUT: (R%d),R%d ; (%02X) = %02X\n",dest, src, app_reg[dest].B[0], val);
        }
    }
    else
    {
        uint8_t port = app_memory[app_pc++];

        if (app_size == 2)
        {
            if ((adr_mode & 0xf0) == 0xF0)
            {
                val = app_reg[adr_mode & 0xf].W[0];
            }
            else
            {
                val = app_reg[0].W[0];
            }
            if (step_mode)
            {
                wprintw(wConsole,"OUT: (%02X) = %04X\n",port,val);
            }
        }
        else
        {
            if ((adr_mode & 0xf0) == 0xF0)
            {
                val = app_reg[adr_mode & 0xf].B[0];
            }
            else
            {
                val = app_reg[0].B[0];
            }
            if (step_mode)
            {
                wprintw(wConsole,"OUT: (%02X) = %02X\n",port,val);
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
                temp = (app_reg[reg].B[j] << 1) | (app_flags & FLAG_C_MASK);
                app_flags = ((temp & 0x100) >> 8) | (app_flags & ~FLAG_C_MASK);
                app_reg[reg].B[j] = temp & 0xff;
            }
        }
    }
    else
    {
        /* right */
        for (i=0;i<nb_shifts;i++)
        {
            /* Get sign bit */
            app_flags = (app_flags & ~FLAG_C_MASK) | ((app_reg[reg].B[app_size-1] & 0x80) >> 7);

            for (j=0;j<app_size;j++)
            {
                temp_cy = (app_reg[reg].B[j] & 0x01);
                temp = ((app_flags & FLAG_C_MASK) << 7) | (app_reg[reg].B[j] >> 1);
                app_flags = temp_cy | (app_flags & ~FLAG_C_MASK);
                app_reg[reg].B[j] = temp & 0xff;
            }
        }
    }
}

/*
 * bit 7       0: left     1: right
 * bits 6-4    nb of shift
 * bits 3-0    reg
 * cy -> bits -> cy
 * cy <- bits <- cy
 */
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
                temp = (app_reg[reg].B[j] << 1) | (app_flags & FLAG_C_MASK);
                app_flags = ((temp & 0x100) >> 8) | (app_flags & ~FLAG_C_MASK);
                app_reg[reg].B[j] = temp & 0xff;
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
                temp_cy = (app_reg[reg].B[j] & 0x01);
                temp = ((app_flags & FLAG_C_MASK) << 7) | (app_reg[reg].B[j] >> 1);
                app_flags = temp_cy | (app_flags & ~FLAG_C_MASK);
                app_reg[reg].B[j] = temp & 0xff;
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
                temp = (app_reg[reg].B[j] << 1) | (app_flags & FLAG_C_MASK);
                app_flags = ((temp & 0x100) >> 8) | (app_flags & ~FLAG_C_MASK);
                app_reg[reg].B[j] = temp & 0xff;
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
                temp_cy = (app_reg[reg].B[j] & 0x01);
                temp = ((app_flags & FLAG_C_MASK) << 7) | (app_reg[reg].B[j] >> 1);
                app_flags = temp_cy | (app_flags & ~FLAG_C_MASK);
                app_reg[reg].B[j] = temp & 0xff;
            }
        }
    }
}

void op_umul(void)
{
    getPair();

    switch(app_size)
    {
    case 1:
        {
            uint16_t result = app_reg[dest].B[0] * app_reg[src].B[0];

            app_reg[src].B[0] = result & 0xff;
            if (result > 256) app_flags |= FLAG_V_MASK;
        }
        break;
    case 2:
        app_reg[dest].W[0] = app_reg[dest].B[0] * app_reg[src].B[0];
        break;
    case 4:
        app_reg[dest].D[0] = app_reg[dest].W[0] * app_reg[src].W[0];
        break;
    case 8:
        app_reg[dest].Q[0] = app_reg[dest].D[0] * app_reg[src].D[0];
        break;
    case 16:
        //app_reg[dest].D[0] = app_reg[dest].W[0] * app_reg[src].W[0];
        //TBD
        break;
    }
}

void op_udiv(void)
{
    getPair();

    switch(app_size)
    {
    case 1:
        if (app_reg[src].B[0] != 0)
        {
            app_reg[dest].B[0] = app_reg[dest].B[0] / app_reg[src].B[0];
        }
        break;
    case 2:
        if (app_reg[src].B[0] != 0)
        {
            app_reg[dest].W[0] = app_reg[dest].W[0] / app_reg[src].B[0];
        }
        break;
    case 4:
        if (app_reg[src].W[0] != 0)
        {
            app_reg[dest].D[0] = app_reg[dest].D[0] / app_reg[src].W[0];
        }
        break;
    case 8:
        if (app_reg[src].D[0] != 0)
        {
            app_reg[dest].Q[0] = app_reg[dest].Q[0] / app_reg[src].D[0];
        }
        break;
    case 16:
        //app_reg[dest].D[0] = app_reg[dest].W[0] * app_reg[src].W[0];
        //TBD
        break;
    }
}

void op_smul(void)
{
    getPair();

    switch(app_size)
    {
    case 1:
        {
            int16_t result = (int8_t) app_reg[dest].B[0] * (int8_t) app_reg[src].B[0];

            app_reg[src].B[0] = result & 0xff;
            if (result > 256) app_flags |= FLAG_V_MASK;
        }
        break;
    case 2:
        app_reg[dest].W[0] = (int8_t) app_reg[dest].B[0] * (int8_t) app_reg[src].B[0];
        break;
    case 4:
        app_reg[dest].D[0] = (int16_t) app_reg[dest].W[0] * (int16_t) app_reg[src].W[0];
        break;
    case 8:
        app_reg[dest].Q[0] = (int32_t) app_reg[dest].D[0] * (int32_t) app_reg[src].D[0];
        break;
    case 16:
        //app_reg[dest].D[0] = app_reg[dest].W[0] * app_reg[src].W[0];
        //TBD
        break;
    }
}

void op_sdiv(void)
{
    getPair();

    switch(app_size)
    {
    case 1:
        if (app_reg[src].B[0] != 0)
        {
            app_reg[dest].B[0] = (int8_t) app_reg[dest].B[0] / (int8_t) app_reg[src].B[0];
        }
        break;
    case 2:
        if (app_reg[src].B[0] != 0)
        {
            app_reg[dest].W[0] = (int16_t) app_reg[dest].W[0] / (int8_t) app_reg[src].B[0];
        }
        break;
    case 4:
        if (app_reg[src].W[0] != 0)
        {
            app_reg[dest].D[0] = (int32_t) app_reg[dest].D[0] / (int16_t) app_reg[src].W[0];
        }
        break;
    case 8:
        if (app_reg[src].D[0] != 0)
        {
            app_reg[dest].Q[0] = (int64_t) app_reg[dest].Q[0] / (int32_t) app_reg[src].D[0];
        }
        break;
    case 16:
        //app_reg[dest].D[0] = app_reg[dest].W[0] * app_reg[src].W[0];
        //TBD
        break;
    }
}

