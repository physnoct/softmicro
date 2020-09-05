#include "softmicro.h"

uint8_t app_memory[65536];
uint8_t app_reg[16][16];
uint8_t app_flags;
int16_t app_pc;
uint8_t app_size,adr_mode;

void illegal_inst(void)
{
}

bool bool_xor(bool a, bool b)
{
    return (a && !b)||(!a && b);
}

uint8_t get_cy(void)
{
    return (app_flags & FLAG_C_MASK);
}

/* Only bit 0 used, others ignored */
void set_cy(uint8_t flags)
{
    app_flags = (app_flags & ~FLAG_C_MASK) | (flags & FLAG_C_MASK);
}

/* flags STHIVNZC
         -----0-*
*/
int8_t byte_add(int8_t a, int8_t b)
{
int16_t x;

    x = a + b;
    set_cy(x>>8);
    app_flags &= ~FLAG_N_MASK;
    return x & 0xff;
}

/* flags STHIVNZC
         -----0-*
*/
int8_t byte_adc(int8_t a, int8_t b)
{
int16_t x;

    x = a + b + get_cy();
    set_cy(x>>8);
    app_flags &= ~FLAG_N_MASK;
    return x & 0xff;
}

/* flags STHIVNZC
         -----1-*
*/
int8_t byte_sub(int8_t a, int8_t b)
{
int16_t x;

    x = a - b;
    if (x != 0) app_flags &= ~FLAG_Z_MASK;
    set_cy(x>>8);
    app_flags |= FLAG_N_MASK;
    return x & 0xff;
}

/* flags STHIVNZC
         -----1-*
*/
int8_t byte_sbc(int8_t a, int8_t b)
{
int16_t x;

    x = a - b - get_cy();
    if (x != 0) app_flags &= ~FLAG_Z_MASK;
    set_cy(x>>8);
    app_flags |= FLAG_N_MASK;
    return x & 0xff;
}

uint8_t swap_byte(uint8_t byte)
{
    return ((byte & 0x0f) << 4) | ((byte & 0xf0) >> 4);
}

uint8_t reverse_byte(uint8_t byte)
{
    return
        ((byte & 0x01) << 7) |
        ((byte & 0x02) << 5) |
        ((byte & 0x04) << 3) |
        ((byte & 0x08) << 1) |
        ((byte & 0x10) >> 1) |
        ((byte & 0x20) >> 3) |
        ((byte & 0x40) >> 5) |
        ((byte & 0x80) >> 7);
}

uint8_t daa_byte(uint8_t byte)
{
uint8_t temp;
    //temp = byte & 0x0f;
    //FIXME
}

/* Convert nibble to ASCII */
uint8_t toa_byte(uint8_t byte)
{
uint8_t temp;
    temp = byte & 0x0f;
    if (temp < 0x0a) return temp | '0';
    else return temp + 'A'-0x0a;
}

/* Convert hex ASCII to nibble */
uint8_t toh_byte(uint8_t byte)
{
    if ((byte >= '0') && (byte < ('9'+1)))
    {
        return byte & 0x0f;
    }
    else
    {
        if ((byte >= 'A') && (byte < 'G'))
        {
            return byte - 'A' + 0x0a;
        }
        else
        {
            if ((byte >= 'a') && (byte < 'g'))
            {
                return byte - 'a' + 0x0a;
            }
            else return byte;
        }
    }
}

uint8_t bcd_byte(uint8_t byte)
{
    if (byte > 99)
    {
        app_flags |= FLAG_V_MASK;
        return 0;
    }
    else
    {
        return (byte/10) << 4 | (byte % 10);
    }
}

uint8_t bin_byte(uint8_t byte)
{
    return ((byte & 0xf0)>>4)*10 + (byte & 0x0f);
}

uint8_t sxt_byte(void)
{
    if ((app_flags & FLAG_C_MASK) != 0) return 0xFF;
    else return 0;
}

/* flags STHIVNZC
         *-----*-
*/
void setflags(uint8_t reg[])
{
int i;
uint8_t zero = 0;
uint8_t sign;

    for (i=1;i<app_size;i++)
    {
        zero |= reg[i];
    }
    sign = reg[app_size-1] & 0x80; // sign flag in bit 7 for convenience
    app_flags = sign | (~zero & FLAG_Z_MASK);
}

void op_load(void)
{
uint8_t pair,src,dest,reg,disp;
int16_t addr;
int i;

    reg = adr_mode & 0x0f;

    switch(adr_mode & 0xf0)
    {
        case 0:
            pair = app_memory[app_pc++];
            src = pair & 0x0f;
            dest = (pair & 0xf0)>>4;

            for (i=0;i<app_size;i++)
            {
                app_reg[dest][i] = app_reg[src][i];
            }
            break;
        case 0x30:
            switch (adr_mode)
            {
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
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src);

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] = app_memory[addr++];
                    }
                    break;

                case 0x39: //r,*p++
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src);

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] = app_memory[addr++];
                    }

                    set_addr(src,addr);
                    break;

                case 0x3A: //r,--*p
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src) - app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] = app_memory[addr++];
                    }
                    set_addr(src,addr);
                    break;

                case 0x3B: //r,p[d] d: index 0-n
                    pair = app_memory[app_pc++];
                    disp = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src) + disp * app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] = app_memory[addr++];
                    }
                    break;

                case 0x3C: //*p,r
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] = app_reg[src][i];
                    }
                    break;

                case 0x3D: //*p++,r
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] = app_reg[src][i];
                    }
                    set_addr(dest,addr);
                    break;

                case 0x3E: //--*p,r
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(dest) - app_size;

                    for (i=0;i<app_size;i++)
                    {
                         app_memory[addr++] = app_reg[src][i];
                    }

                    set_addr(dest,addr);
                    break;

                case 0x3F: //p[d],r d: index 0-n
                    pair = app_memory[app_pc++];
                    disp = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

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

#if 0
        //r': adr pointer, d: 8-bit displacement
        case 0xB0: //r,(r')
            addr = get_addr(src);

            for (i=0;i<app_size;i++)
            {
                app_reg[reg][i] = app_memory[addr++];
            }
            break;
        case 0xC0: //r,(r'+d)
            break;
        case 0xD0: //(r'),r
            addr = get_addr(dest);

            for (i=0;i<app_size;i++)
            {
                app_memory[addr++] = app_reg[reg][i];
            }
            break;
            break;
        case 0xE0: //(r'+d),r
            break;
#endif
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

int16_t ex_reg_memory(uint8_t addr, uint8_t reg)
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
uint8_t pair,src,dest,reg,disp,temp;
int16_t addr;
int i;

    reg = adr_mode & 0x0f;

    switch(adr_mode & 0xf0)
    {
        case 0:
            pair = app_memory[app_pc++];
            src = pair & 0x0f;
            dest = (pair & 0xf0)>>4;

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
                case 0x3c:
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;
                    addr = get_addr(src);
                    ex_reg_memory(addr,dest);
                    break;

                case 0x39: //r,*p++
                case 0x3d:
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src);
                    addr = ex_reg_memory(addr,dest);
                    set_addr(src,addr);
                    break;

                case 0x3A: //r,--*p
                case 0x3e:
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src) - app_size;
                    addr = ex_reg_memory(addr,dest);
                    set_addr(src,addr);
                    break;

                case 0x3B: //r,p[d] d: index 0-n
                case 0x3f:
                    pair = app_memory[app_pc++];
                    disp = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src) + disp * app_size;
                    ex_reg_memory(addr,dest);
                    break;

                default:
                    illegal_inst();
            }
            break;

#if 0
        //r': adr pointer, d: 8-bit displacement
        case 0xB0: //r,(r')
            addr = get_addr(src);

            for (i=0;i<app_size;i++)
            {
                app_reg[reg][i] = app_memory[addr++];
            }
            break;
        case 0xC0: //r,(r'+d)
            break;
        case 0xD0: //(r'),r
            addr = get_addr(dest);

            for (i=0;i<app_size;i++)
            {
                app_memory[addr++] = app_reg[reg][i];
            }
            break;
            break;
        case 0xE0: //(r'+d),r
            break;
#endif
        default:
            illegal_inst();
    }
}

void op_add(void)
{
uint8_t pair,src,dest,reg,disp,temp;
int16_t addr;
int i;

    reg = adr_mode & 0x0f;

    switch(adr_mode & 0xf0)
    {
        case 0:
            pair = app_memory[app_pc++];
            src = pair & 0x0f;
            dest = (pair & 0xf0)>>4;

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
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src);

                    app_reg[dest][0] = byte_add(app_reg[dest][0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_adc(app_reg[dest][i],app_memory[addr++]);
                    }
                    break;

                case 0x39: //r,*p++
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src);

                    app_reg[dest][0] = byte_add(app_reg[dest][0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_adc(app_reg[dest][i],app_memory[addr++]);
                    }

                    set_addr(src,addr);
                    break;

                case 0x3A: //r,--*p
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src) - app_size;

                    app_reg[dest][0] = byte_add(app_reg[dest][0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_adc(app_reg[dest][i],app_memory[addr++]);
                    }
                    set_addr(src,addr);
                    break;

                case 0x3B: //r,p[d] d: index 0-n
                    pair = app_memory[app_pc++];
                    disp = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src) + disp * app_size;

                    app_reg[dest][0] = byte_add(app_reg[dest][0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_adc(app_reg[dest][i],app_memory[addr++]);
                    }
                    break;

                case 0x3C: //*p,r
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

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
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

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
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

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
                    pair = app_memory[app_pc++];
                    disp = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

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

#if 0
        //r': adr pointer, d: 8-bit displacement
        case 0xB0: //r,(r')
            addr = get_addr(src);

            app_reg[reg][0] = byte_add(app_reg[reg][0],app_memory[addr++]);

            for (i=1;i<app_size;i++)
            {
                app_reg[reg][i] = byte_adc(app_reg[reg][i],app_memory[addr++]);
            }
            break;
        case 0xC0: //r,(r'+d)
            break;
        case 0xD0: //(r'),r
            addr = get_addr(dest);

            app_memory[addr] = byte_add(app_memory[addr],app_reg[src][0]);
            addr++;

            for (i=1;i<app_size;i++)
            {
                app_memory[addr] = byte_adc(app_memory[addr],app_reg[src][i]);
                addr++;
            }
            break;
            break;
        case 0xE0: //(r'+d),r
            break;
#endif
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
uint8_t pair,src,dest,reg,disp,temp;
int16_t addr;
int i;

    reg = adr_mode & 0x0f;

    switch(adr_mode & 0xf0)
    {
        case 0:
            pair = app_memory[app_pc++];
            src = pair & 0x0f;
            dest = (pair & 0xf0)>>4;

            for (i=0;i<app_size;i++)
            {
                app_reg[dest][i] = byte_adc(app_reg[dest][i],app_reg[src][i]);
            }
            setflags(app_reg[dest]);
            break;
        case 0x30:
            switch (adr_mode)
            {
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
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src);

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_adc(app_reg[dest][i],app_memory[addr++]);
                    }
                    break;

                case 0x39: //r,*p++
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src);

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_adc(app_reg[dest][i],app_memory[addr++]);
                    }

                    set_addr(src,addr);
                    break;

                case 0x3A: //r,--*p
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src) - app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_adc(app_reg[dest][i],app_memory[addr++]);
                    }
                    set_addr(src,addr);
                    break;

                case 0x3B: //r,p[d] d: index 0-n
                    pair = app_memory[app_pc++];
                    disp = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src) + disp * app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_adc(app_reg[dest][i],app_memory[addr++]);
                    }
                    break;

                case 0x3C: //*p,r
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr] = byte_adc(app_memory[addr],app_reg[src][i]);
                        addr++;
                    }
                    break;

                case 0x3D: //*p++,r
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr] = byte_adc(app_memory[addr],app_reg[src][i]);
                        addr++;
                    }
                    set_addr(dest,addr);
                    break;

                case 0x3E: //--*p,r
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(dest) - app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr] = byte_adc(app_memory[addr],app_reg[src][i]);
                        addr++;
                    }

                    set_addr(dest,addr);
                    break;

                case 0x3F: //p[d],r d: index 0-n
                    pair = app_memory[app_pc++];
                    disp = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

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

#if 0
        //r': adr pointer, d: 8-bit displacement
        case 0xB0: //r,(r')
            addr = get_addr(src);

            for (i=0;i<app_size;i++)
            {
                app_reg[reg][i] = byte_adc(app_reg[reg][i],app_memory[addr++]);
            }
            break;
        case 0xC0: //r,(r'+d)
            break;
        case 0xD0: //(r'),r
            addr = get_addr(dest);

            for (i=0;i<app_size;i++)
            {
                app_memory[addr] = byte_adc(app_memory[addr],app_reg[src][i]);
                addr++;
            }
            break;
            break;
        case 0xE0: //(r'+d),r
            break;
#endif
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
uint8_t pair,src,dest,reg,disp,temp;
int16_t addr;
int i;

    reg = adr_mode & 0x0f;

    switch(adr_mode & 0xf0)
    {
        case 0:
            pair = app_memory[app_pc++];
            src = pair & 0x0f;
            dest = (pair & 0xf0)>>4;

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
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src);

                    app_reg[dest][0] = byte_sub(app_reg[dest][0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_sbc(app_reg[dest][i],app_memory[addr++]);
                    }
                    break;

                case 0x39: //r,*p++
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src);

                    app_reg[dest][0] = byte_sub(app_reg[dest][0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_sbc(app_reg[dest][i],app_memory[addr++]);
                    }

                    set_addr(src,addr);
                    break;

                case 0x3A: //r,--*p
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src) - app_size;

                    app_reg[dest][0] = byte_sub(app_reg[dest][0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_sbc(app_reg[dest][i],app_memory[addr++]);
                    }
                    set_addr(src,addr);
                    break;

                case 0x3B: //r,p[d] d: index 0-n
                    pair = app_memory[app_pc++];
                    disp = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src) + disp * app_size;

                    app_reg[dest][0] = byte_sub(app_reg[dest][0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_sbc(app_reg[dest][i],app_memory[addr++]);
                    }
                    break;

                case 0x3C: //*p,r
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

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
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

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
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

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
                    pair = app_memory[app_pc++];
                    disp = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

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

#if 0
        //r': adr pointer, d: 8-bit displacement
        case 0xB0: //r,(r')
            addr = get_addr(src);

            app_reg[reg][0] = byte_sub(app_reg[reg][0],app_memory[addr++]);

            for (i=1;i<app_size;i++)
            {
                app_reg[reg][i] = byte_sbc(app_reg[reg][i],app_memory[addr++]);
            }
            break;
        case 0xC0: //r,(r'+d)
            break;
        case 0xD0: //(r'),r
            addr = get_addr(dest);

            app_memory[addr] = byte_sub(app_memory[addr],app_reg[src][0]);
            addr++;

            for (i=1;i<app_size;i++)
            {
                app_memory[addr] = byte_sbc(app_memory[addr],app_reg[src][i]);
                addr++;
            }
            break;
            break;
        case 0xE0: //(r'+d),r
            break;
#endif
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
uint8_t pair,src,dest,reg,disp,temp;
int16_t addr;
int i;

    reg = adr_mode & 0x0f;

    switch(adr_mode & 0xf0)
    {
        case 0:
            pair = app_memory[app_pc++];
            src = pair & 0x0f;
            dest = (pair & 0xf0)>>4;

            for (i=0;i<app_size;i++)
            {
                app_reg[dest][i] = byte_sbc(app_reg[dest][i],app_reg[src][i]);
            }
            setflags(app_reg[dest]);
            break;
        case 0x30:
            switch (adr_mode)
            {
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
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src);

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_sbc(app_reg[dest][i],app_memory[addr++]);
                    }
                    break;

                case 0x39: //r,*p++
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src);

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_sbc(app_reg[dest][i],app_memory[addr++]);
                    }

                    set_addr(src,addr);
                    break;

                case 0x3A: //r,--*p
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src) - app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_sbc(app_reg[dest][i],app_memory[addr++]);
                    }
                    set_addr(src,addr);
                    break;

                case 0x3B: //r,p[d] d: index 0-n
                    pair = app_memory[app_pc++];
                    disp = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src) + disp * app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] = byte_sbc(app_reg[dest][i],app_memory[addr++]);
                    }
                    break;

                case 0x3C: //*p,r
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr] = byte_sbc(app_memory[addr],app_reg[src][i]);
                        addr++;
                    }
                    break;

                case 0x3D: //*p++,r
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr] = byte_sbc(app_memory[addr],app_reg[src][i]);
                        addr++;
                    }
                    set_addr(dest,addr);
                    break;

                case 0x3E: //--*p,r
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(dest) - app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr] = byte_sbc(app_memory[addr],app_reg[src][i]);
                        addr++;
                    }

                    set_addr(dest,addr);
                    break;

                case 0x3F: //p[d],r d: index 0-n
                    pair = app_memory[app_pc++];
                    disp = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

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

#if 0
        //r': adr pointer, d: 8-bit displacement
        case 0xB0: //r,(r')
            addr = get_addr(src);

            for (i=0;i<app_size;i++)
            {
                app_reg[reg][i] = byte_sbc(app_reg[reg][i],app_memory[addr++]);
            }
            break;
        case 0xC0: //r,(r'+d)
            break;
        case 0xD0: //(r'),r
            addr = get_addr(dest);

            for (i=0;i<app_size;i++)
            {
                app_memory[addr] = byte_sbc(app_memory[addr],app_reg[src][i]);
                addr++;
            }
            break;
            break;
        case 0xE0: //(r'+d),r
            break;
#endif
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
uint8_t pair,src,dest,reg,disp,temp;
int16_t addr;
int i;

    app_flags |= FLAG_Z_MASK;

    reg = adr_mode & 0x0f;

    switch(adr_mode & 0xf0)
    {
        case 0:
            pair = app_memory[app_pc++];
            src = pair & 0x0f;
            dest = (pair & 0xf0)>>4;

            byte_sub(app_reg[dest][0],app_reg[src][0]);

            for (i=1;i<app_size;i++)
            {
                byte_sbc(app_reg[dest][i],app_reg[src][i]);
            }
            break;
        case 0x30:
            switch (adr_mode)
            {
                case 0x37: //ADDR,#
                    // Read addr
                    addr = getword(app_pc);
                    app_pc +=2;

                    byte_sub(app_memory[addr],app_memory[app_pc++]);
                    addr++;

                    //Imm to memory
                    for (i=1;i<app_size;i++)
                    {
                        byte_sbc(app_memory[addr],app_memory[app_pc++]);
                        addr++;
                    }
                    break;

                case 0x38: //r,*p
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src);

                    byte_sub(app_reg[dest][0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        byte_sbc(app_reg[dest][i],app_memory[addr++]);
                    }
                    break;

                case 0x39: //r,*p++
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src);

                    byte_sub(app_reg[dest][0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        byte_sbc(app_reg[dest][i],app_memory[addr++]);
                    }

                    set_addr(src,addr);
                    break;

                case 0x3A: //r,--*p
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src) - app_size;

                    byte_sub(app_reg[dest][0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        byte_sbc(app_reg[dest][i],app_memory[addr++]);
                    }
                    set_addr(src,addr);
                    break;

                case 0x3B: //r,p[d] d: index 0-n
                    pair = app_memory[app_pc++];
                    disp = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src) + disp * app_size;

                    byte_sub(app_reg[dest][0],app_memory[addr++]);

                    for (i=1;i<app_size;i++)
                    {
                        byte_sbc(app_reg[dest][i],app_memory[addr++]);
                    }
                    break;

                case 0x3C: //*p,r
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(dest);

                    byte_sub(app_memory[addr],app_reg[src][0]);
                    addr++;

                    for (i=1;i<app_size;i++)
                    {
                        byte_sbc(app_memory[addr],app_reg[src][i]);
                        addr++;
                    }
                    break;

                case 0x3D: //*p++,r
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(dest);

                    byte_sub(app_memory[addr],app_reg[src][0]);
                    addr++;

                    for (i=1;i<app_size;i++)
                    {
                        byte_sbc(app_memory[addr],app_reg[src][i]);
                        addr++;
                    }
                    set_addr(dest,addr);
                    break;

                case 0x3E: //--*p,r
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(dest) - app_size;

                    byte_sub(app_memory[addr],app_reg[src][0]);
                    addr++;

                    for (i=1;i<app_size;i++)
                    {
                        byte_sbc(app_memory[addr],app_reg[src][i]);
                        addr++;
                    }

                    set_addr(dest,addr);
                    break;

                case 0x3F: //p[d],r d: index 0-n
                    pair = app_memory[app_pc++];
                    disp = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(dest) + disp * app_size;

                    byte_sub(app_memory[addr],app_reg[src][0]);
                    addr++;

                    for (i=1;i<app_size;i++)
                    {
                        byte_sbc(app_memory[addr],app_reg[src][i]);
                        addr++;
                    }
                    break;

                default:
                    illegal_inst();
            }
            break;

#if 0
        //r': adr pointer, d: 8-bit displacement
        case 0xB0: //r,(r')
            addr = get_addr(src);

            byte_sub(app_reg[reg][0],app_memory[addr++]);

            for (i=1;i<app_size;i++)
            {
                byte_sbc(app_reg[reg][i],app_memory[addr++]);
            }
            break;
        case 0xC0: //r,(r'+d)
            break;
        case 0xD0: //(r'),r
            addr = get_addr(dest);

            byte_sub(app_memory[addr],app_reg[src][0]);
            addr++;

            for (i=1;i<app_size;i++)
            {
                byte_sbc(app_memory[addr],app_reg[src][i]);
                addr++;
            }
            break;
            break;
        case 0xE0: //(r'+d),r
            break;
#endif
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
uint8_t pair,src,dest,reg,disp,temp;
int16_t addr;
int i;

    reg = adr_mode & 0x0f;

    switch(adr_mode & 0xf0)
    {
        case 0:
            pair = app_memory[app_pc++];
            src = pair & 0x0f;
            dest = (pair & 0xf0)>>4;

            for (i=0;i<app_size;i++)
            {
                app_reg[dest][i] &= app_reg[src][i];
            }
            setflags(app_reg[dest]);
            break;
        case 0x30:
            switch (adr_mode)
            {
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
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src);

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] &= app_memory[addr++];
                    }
                    break;

                case 0x39: //r,*p++
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src);

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] &= app_memory[addr++];
                    }

                    set_addr(src,addr);
                    break;

                case 0x3A: //r,--*p
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src) - app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] &= app_memory[addr++];
                    }
                    set_addr(src,addr);
                    break;

                case 0x3B: //r,p[d] d: index 0-n
                    pair = app_memory[app_pc++];
                    disp = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src) + disp * app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] &= app_memory[addr++];
                    }
                    break;

                case 0x3C: //*p,r
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] &= app_reg[src][i];
                    }
                    break;

                case 0x3D: //*p++,r
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] &= app_reg[src][i];
                    }
                    set_addr(dest,addr);
                    break;

                case 0x3E: //--*p,r
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(dest) - app_size;

                    for (i=0;i<app_size;i++)
                    {
                         app_memory[addr++] &= app_reg[src][i];
                    }

                    set_addr(dest,addr);
                    break;

                case 0x3F: //p[d],r d: index 0-n
                    pair = app_memory[app_pc++];
                    disp = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

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

#if 0
        //r': adr pointer, d: 8-bit displacement
        case 0xB0: //r,(r')
            addr = get_addr(src);

            for (i=0;i<app_size;i++)
            {
                app_reg[reg][i] &= app_memory[addr++];
            }
            break;
        case 0xC0: //r,(r'+d)
            break;
        case 0xD0: //(r'),r
            addr = get_addr(dest);

            for (i=0;i<app_size;i++)
            {
                app_memory[addr++] &= app_reg[reg][i];
            }
            break;
            break;
        case 0xE0: //(r'+d),r
            break;
#endif
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
uint8_t pair,src,dest,reg,disp,temp;
int16_t addr;
int i;

    reg = adr_mode & 0x0f;

    switch(adr_mode & 0xf0)
    {
        case 0:
            pair = app_memory[app_pc++];
            src = pair & 0x0f;
            dest = (pair & 0xf0)>>4;

            for (i=0;i<app_size;i++)
            {
                app_reg[dest][i] |= app_reg[src][i];
            }
            setflags(app_reg[dest]);
            break;
        case 0x30:
            switch (adr_mode)
            {
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
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src);

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] |= app_memory[addr++];
                    }
                    break;

                case 0x39: //r,*p++
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src);

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] |= app_memory[addr++];
                    }

                    set_addr(src,addr);
                    break;

                case 0x3A: //r,--*p
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src) - app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] |= app_memory[addr++];
                    }
                    set_addr(src,addr);
                    break;

                case 0x3B: //r,p[d] d: index 0-n
                    pair = app_memory[app_pc++];
                    disp = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src) + disp * app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] |= app_memory[addr++];
                    }
                    break;

                case 0x3C: //*p,r
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] |= app_reg[src][i];
                    }
                    break;

                case 0x3D: //*p++,r
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] |= app_reg[src][i];
                    }
                    set_addr(dest,addr);
                    break;

                case 0x3E: //--*p,r
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(dest) - app_size;

                    for (i=0;i<app_size;i++)
                    {
                         app_memory[addr++] |= app_reg[src][i];
                    }

                    set_addr(dest,addr);
                    break;

                case 0x3F: //p[d],r d: index 0-n
                    pair = app_memory[app_pc++];
                    disp = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

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

#if 0
        //r': adr pointer, d: 8-bit displacement
        case 0xB0: //r,(r')
            addr = get_addr(src);

            for (i=0;i<app_size;i++)
            {
                app_reg[reg][i] |= app_memory[addr++];
            }
            break;
        case 0xC0: //r,(r'+d)
            break;
        case 0xD0: //(r'),r
            addr = get_addr(dest);

            for (i=0;i<app_size;i++)
            {
                app_memory[addr++] |= app_reg[reg][i];
            }
            break;
            break;
        case 0xE0: //(r'+d),r
            break;
#endif
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
uint8_t pair,src,dest,reg,disp,temp;
int16_t addr;
int i;

    reg = adr_mode & 0x0f;

    switch(adr_mode & 0xf0)
    {
        case 0:
            pair = app_memory[app_pc++];
            src = pair & 0x0f;
            dest = (pair & 0xf0)>>4;

            for (i=0;i<app_size;i++)
            {
                app_reg[dest][i] ^= app_reg[src][i];
            }
            setflags(app_reg[dest]);
            break;
        case 0x30:
            switch (adr_mode)
            {
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
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src);

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] ^= app_memory[addr++];
                    }
                    break;

                case 0x39: //r,*p++
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src);

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] ^= app_memory[addr++];
                    }

                    set_addr(src,addr);
                    break;

                case 0x3A: //r,--*p
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src) - app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] ^= app_memory[addr++];
                    }
                    set_addr(src,addr);
                    break;

                case 0x3B: //r,p[d] d: index 0-n
                    pair = app_memory[app_pc++];
                    disp = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(src) + disp * app_size;

                    for (i=0;i<app_size;i++)
                    {
                        app_reg[dest][i] ^= app_memory[addr++];
                    }
                    break;

                case 0x3C: //*p,r
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] ^= app_reg[src][i];
                    }
                    break;

                case 0x3D: //*p++,r
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(dest);

                    for (i=0;i<app_size;i++)
                    {
                        app_memory[addr++] ^= app_reg[src][i];
                    }
                    set_addr(dest,addr);
                    break;

                case 0x3E: //--*p,r
                    pair = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

                    addr = get_addr(dest) - app_size;

                    for (i=0;i<app_size;i++)
                    {
                         app_memory[addr++] ^= app_reg[src][i];
                    }

                    set_addr(dest,addr);
                    break;

                case 0x3F: //p[d],r d: index 0-n
                    pair = app_memory[app_pc++];
                    disp = app_memory[app_pc++];
                    src = pair & 0x0f;
                    dest = (pair & 0xf0)>>4;

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

#if 0
        //r': adr pointer, d: 8-bit displacement
        case 0xB0: //r,(r')
            addr = get_addr(src);

            for (i=0;i<app_size;i++)
            {
                app_reg[reg][i] ^= app_memory[addr++];
            }
            break;
        case 0xC0: //r,(r'+d)
            break;
        case 0xD0: //(r'),r
            addr = get_addr(dest);

            for (i=0;i<app_size;i++)
            {
                app_memory[addr++] ^= app_reg[reg][i];
            }
            break;
            break;
        case 0xE0: //(r'+d),r
            break;
#endif
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
bool cy;

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
bool cy;

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
    printf("IN: [%02X] = ",port);

    result = scanf("%02X",&val);
    printf("Result: %02X, Input: %02X\n",result,val & 0xff);

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
    printf("OUT: [%02X] = %02X\n",port,val);
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

    printf("ASX Side: %d, Shifts: %d, Reg: %d\n",side,nb_shifts,reg);

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
                app_flags = ((temp & 0x100) >> 8) | (app_flags &= ~FLAG_C_MASK);
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
                app_flags = temp_cy | (app_flags &= ~FLAG_C_MASK);
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

    printf("ROX Side: %d, Shifts: %d, Reg: %d\n",side,nb_shifts,reg);

    if (side == 0)
    {
        /* left */
        for (i=0;i<nb_shifts;i++)
        {
            for (j=0;j<app_size;j++)
            {
                temp = (app_reg[reg][j] << 1) | (app_flags & FLAG_C_MASK);
                app_flags = ((temp & 0x100) >> 8) | (app_flags &= ~FLAG_C_MASK);
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
                app_flags = temp_cy | (app_flags &= ~FLAG_C_MASK);
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

    printf("LSX Side: %d, Shifts: %d, Reg: %d\n",side,nb_shifts,reg);

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
                app_flags = ((temp & 0x100) >> 8) | (app_flags &= ~FLAG_C_MASK);
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
                app_flags = temp_cy | (app_flags &= ~FLAG_C_MASK);
                app_reg[reg][j] = temp & 0xff;
            }
        }
    }
}

/* reg = 0 to F */
int16_t get_addr(uint8_t reg)
{
    return app_reg[reg][1]*256 + app_reg[reg][0];
}

/* reg = 0 to F */
int16_t get_addr_disp8(uint8_t reg,uint8_t disp8)
{
int16_t disp16;

    disp16 = disp8;
    if (disp8 & 0x80) disp16 |= 0xFF00;
    return get_addr(reg) + disp16;
}

/* reg = 0 to F */
void set_addr(uint8_t reg, int16_t addr)
{
    app_reg[reg][1] = (addr >> 8) & 0xff;
    app_reg[reg][0] = addr & 0xff;
}

/* Used for branch instructions. When size > 2, illegal instruction */
int16_t get_disp(void)
{
int16_t disp16;

    switch (app_size)
    {
        case 2:
            disp16 = getword(app_pc);
            break;
        case 1:
            disp16 = app_memory[app_pc];
            if (disp16 & 0x80) disp16 |= 0xFF00;
            break;
        default:
            illegal_inst();
    }
    printf("get_disp: Size: %d, app_pc: %04X, disp = %04X\n",app_size,app_pc & 0xffff,disp16 & 0xFFFF);
    return disp16;
}

int16_t getword(uint16_t addr)
{
int16_t word;

    word = app_memory[addr+1]*256 + app_memory[addr];
    return word;
}

int16_t getsp(void)
{
    return app_reg[15][1]*256 + app_reg[15][0];
}

void setsp(int16_t value)
{
    app_reg[15][1] = (value >> 8) & 0xff;
    app_reg[15][0] = value & 0xff;
}

void put_retaddr(int16_t value)
{
int16_t sp;

    sp = getsp();
    app_memory[sp-1] = (value >> 8) & 0xff;
    app_memory[sp-2] = value & 0xff;
    setsp(sp-2);
    printf("value: %04X, sp: %04X, H: %02X, L: %02X\n",value,sp,app_memory[sp-1],app_memory[sp-2]);
}

int16_t get_retaddr(void)
{
int16_t sp;

    sp = getsp();
    setsp(sp+2);
    return app_memory[sp+1]*256 + app_memory[sp];
}

void push_byte(uint8_t my_byte)
{
uint16_t sp;

    sp = (getsp() - 1) & 0xffff;

    printf("push_byte:\tSP: %04X\n",sp);
    app_memory[sp] = my_byte;
    setsp(sp);
}

uint8_t pop_byte(void)
{
uint16_t sp;
uint8_t my_byte;

    sp = getsp();
    my_byte = app_memory[sp];
    setsp(sp+1);

    return my_byte;
}


void branch(uint8_t param)
{
uint8_t test_bit,test_mask;
bool test;

    // Bit 3-1  flag
    // Bit 0    0:clear 1:set
    test_bit = (param & 0x0E) >> 1;
    test_mask = 1 << test_bit;
    test = param & 0x01;

    if ((app_flags & test_mask) == test)
    {
        app_pc += get_disp();
    }
    else
    {
        app_pc += app_size;
    }
}

void branch2(uint8_t opcode)
{
bool test;
bool n,v;

    test = ((opcode & 0x10) == 0x10);
    //BRGE	N ⊕ V = 0 0x01
    //BRLT	N ⊕ V = 1 0x31
    n = ((app_flags & FLAG_N_MASK) == FLAG_N_MASK);
    v = ((app_flags & FLAG_V_MASK) == FLAG_V_MASK);

    if (bool_xor(n,v) == test)
    {
        app_pc += get_disp();
    }
    else
    {
        app_pc += app_size;
    }
}

void br_sbr(void)
{
int16_t temp;

    put_retaddr(app_pc + app_size); // next instruction after call
    temp = get_disp();
    app_pc += temp;
    printf("BSR %04X (%04X)\n",temp, app_pc);
}

void br_always(void)
{
int16_t temp;

    temp = get_disp();
    printf("BR %04X (%04X) (%04X)\n",temp & 0xFFFF, app_pc & 0xFFFF, (app_pc+app_size+temp) & 0xFFFF);
    app_pc = app_pc + app_size + temp;
}

/* Wait until port bit n = 0
Once instruction is read, execute a port read until condition is met */
void wait_port(uint8_t param)
{
uint8_t bit,value,mask,port;
int input;
int result;

    bit = param & 0x07;
    mask = (1 << bit);
    value = ((param & 0x08) >> 3) << bit;
    port = app_memory[app_pc++];

    do
    {
        printf("WAIT PORT,BIT,VALUE: [%02X.%d] = %d\n",port,bit,(param & 0x08) >> 3);
        result = scanf("%02X",&input);
        printf("Result: %02X, Input: %02X\n",result,input & 0xff);
        printf("Mask: %02X, Value: %02X\n",mask,value);
        if ((input & mask) == value) printf("Bit match\n");
    } while ((input & mask) != value);
}

void app_vector(uint8_t param)
{
uint16_t addr;
    put_retaddr(app_pc);
    addr = VECTOR_TABLE_BEGIN + (param << 1);
    app_pc = getword(addr);
}

void djnz(uint8_t param)
{
int16_t temp;

    temp = get_disp();
    printf("DJNZ %04X (%04X) (%04X)\n",temp & 0xFFFF, app_pc & 0xFFFF, (app_pc+app_size+temp) & 0xFFFF);

    app_reg[param][0] -= 1;

    if (app_reg[param][0] == 0) app_pc = app_pc + app_size;
    else app_pc = app_pc + app_size + temp;
}

/* flags STHIVNZC
         0-0-0010
*/
void clr(uint8_t param)
{
int i;

    if (adr_mode == 0)
    {
        for (i=0;i<app_size;i++)
        {
            app_reg[param][i] = 0;
        }
    }
    else
    {
    }
    app_flags &= 0x50;
    app_flags |= 0x02;
}

/* flags STHIVNZC
         --------
*/
void inc(uint8_t param)
{
    if (adr_mode == 0)
    {
        op_inc(app_reg[param]);
    }
    else
    {
    }
}

void dec(uint8_t param)
{
int i;

    if (adr_mode == 0)
    {
        op_dec(app_reg[param]);
    }
    else
    {
    }
}

uint8_t set_bit_val(bool val, uint8_t bit, uint8_t reg)
{
uint8_t temp, mask;

    mask = 1<<bit;
    temp = reg & ~mask;     //clr bit
    if (val) temp |= mask;  //if val = 1 set bit
    return temp;
}

uint8_t get_bit_val(uint8_t bit, uint8_t reg)
{
uint8_t mask;

    mask = 1<<bit;
    return reg & mask;
}

void test_bit_reg(uint8_t bit, uint8_t reg)
{
uint8_t mask;

    mask = 1<<bit;

    app_flags &= ~FLAG_Z_MASK;
    if (reg ^ mask) app_flags |= FLAG_Z_MASK;
}

/* Size = 1 only */
void set_bit(void)
{
uint8_t pair,bit,reg;
uint16_t addr,disp16;
bool val;

    if (app_size != 1) illegal_inst();

    pair = app_memory[app_pc];
    reg = pair & 0x0f;
    bit = (pair & 0x70) >> 4;
    val = ((pair & 0x80) == 0x80);

    switch(adr_mode & 0xF0)
    {
        case 0:
            app_reg[reg][0] = set_bit_val(val,bit,app_reg[reg][0]);
            app_pc++;
            break;
        default:
            illegal_inst();
    }
}

/* Size = 1 only */
/* bit 7 of following byte is unused */
void toggle_bit(void)
{
uint8_t pair,bit,reg, mask;

    pair = app_memory[app_pc];
    reg = pair & 0x0f;
    bit = (pair & 0x70) >> 4;
    mask = 1<<bit;

    switch(adr_mode & 0xF0)
    {
        case 0:
            app_reg[reg][0] ^= mask;
            app_pc++;
            break;
        default:
            illegal_inst();
    }
}

/* Size = 1 only */
/* Set Z flag if bit = 0*/
void test_bit(void)
{
uint8_t pair,bit,reg;

    pair = app_memory[app_pc];
    reg = pair & 0x0f;
    bit = (pair & 0x70) >> 4;

    switch(adr_mode & 0xF0)
    {
        case 0:
            test_bit_reg(app_reg[reg][0],bit);
            app_pc++;
            break;
        default:
            illegal_inst();
    }
}

/* Size = 1 only */
/* Waiting for a bit changed while in interrupt/by an interrupt */
void wait_bit(void)
{
uint8_t pair,bit,reg,val;

    pair = app_memory[(app_pc++) & 0xffff];
    reg = pair & 0x0f;
    bit = (pair & 0x70) >> 4;
    val = pair & 0x80;

    switch(adr_mode & 0xF0)
    {
        case 0:
            if (val != get_bit_val(bit,app_reg[reg][0]))
            {
                printf("Waiting for bit %d of reg %d to be %d\n",bit,reg,val>>7);
                getchar();
            }
            break;
        default:
            illegal_inst();
    }
}

void OpStep(void)
{
uint8_t op_code,param;
int16_t temp;

    app_size = 1;
    adr_mode = 0;

    op_code = app_memory[app_pc++];

    printf("OpStep\tSize: %d, Addr mode: %02X, Op: %02X\n",app_size,adr_mode,op_code);

    if (op_code >= 0x40)
    {
        if  (op_code >= 0xB0)
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
                printf("CALL %04X, SP: %04X, Ret Addr %04X\n",app_pc,temp & 0xFFFF,app_memory[temp+1]*256+app_memory[temp]);
                break;
            case 0x09: //BSR
                br_sbr();
                break;
            case 0x0a: //JMP
                app_pc = getword(app_pc);
                printf("JMP %04X\n",app_pc);
                break;
            case 0x0b: //BR
                br_always();
                break;

            /* Size prefix */
            case 0x0c: //.W
                app_size = 2;
                printf(".W\n");
                OpStep2();
                break;
            case 0x0d: //.D
                app_size = 4;
                printf(".D\n");
                OpStep2();
                break;
            case 0x0e: //.Q
                app_size = 8;
                printf(".Q\n");
                OpStep2();
                break;
            case 0x0f: //.O
                app_size = 16;
                printf(".O\n");
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
            case 0x20: // SWAP
                app_reg[0][0] = swap_byte(app_reg[0][0]);
                break;
            case 0x21: // REV
                app_reg[0][0] = reverse_byte(app_reg[0][0]);
                break;
            case 0x22: // DAA
                app_reg[0][0] = daa_byte(app_reg[0][0]);
                break;
            case 0x23: // TOA
                app_reg[0][0] = toa_byte(app_reg[0][0]);
                break;
            case 0x24: // TOH
                app_reg[0][0] = toh_byte(app_reg[0][0]);
                break;
            case 0x25: // BCD
                app_reg[0][0] = bcd_byte(app_reg[0][0]);
                break;
            case 0x26: // BIN
                app_reg[0][0] = bin_byte(app_reg[0][0]);
                break;
            case 0x27: // SXT
                app_reg[0][0] = sxt_byte();
                break;
            case 0x28: // CPL
                app_reg[0][0] = ~app_reg[0][0];
                break;
            case 0x29: // NEG
                app_reg[0][0] = -app_reg[0][0];
                break;
            case 0x2a: // LDF
                app_reg[0][0] = app_flags;
                break;
            case 0x2b: // STF
                app_flags = app_reg[0][0];
                break;
            case 0x2c: // MSK
                app_reg[0][0] = 1 << (app_reg[0][0] & 0x07);
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
int16_t temp;

    op_code = app_memory[app_pc++];

    printf("OpStep2\tSize: %d, Addr mode: %02X, Op: %02X\n",app_size,adr_mode,op_code);

    if (op_code >= 0x40)
    {
        if  (op_code >= 0xB0)
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
int16_t temp;

    op_code = app_memory[app_pc++];

    printf("OpStep3\tSize: %d, Addr mode: %02X, Op: %02X\n",app_size,adr_mode,op_code);

    if (op_code >= 0x70)
    {
        if  (op_code >= 0xB0)
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
                case 0x80:
                    clr(param);
                    break;
                case 0x90:
                    inc(param);
                    break;
                case 0xA0:
                    dec(param);
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
            default:
                illegal_inst();
        }
    }
}

void OpExtended(void)
{
uint8_t op_code; //,param;

    op_code = app_memory[app_pc];

    printf("OpExtended\tSize: %d, Addr mode: %02X, Op: %02X\n",app_size,adr_mode,op_code);

    switch(op_code)
    {
        case 0x00: //VER
            app_pc++;
            app_reg[0][0] = 1;
            break;
        case 0x01: //SN
            app_pc++;
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
            printf("Halt\n");
            break;
        case 0x03: // CLR H
            app_flags &= ~FLAG_H_MASK;
            app_pc++;
            break;
        case 0x04: // SET H
            app_flags |= FLAG_H_MASK;
            app_pc++;
            break;
        case 0x05: // TOG H
            app_flags ^= FLAG_H_MASK;
            app_pc++;
            break;
        case 0x06: // CLR T
            app_flags &= ~FLAG_T_MASK;
            app_pc++;
            break;
        case 0x07: // SET T
            app_flags |= FLAG_T_MASK;
            app_pc++;
            break;
        case 0x08: // TOG T
            app_flags ^= FLAG_T_MASK;
            app_pc++;
            break;
        default:
            illegal_inst();
    }
}
