#include "softmicro.h"

uint8_t reg_pair,src,dest;

void getPair(void)
{
    reg_pair = app_memory[app_pc++];
    src = reg_pair & 0x0f;
    dest = (reg_pair & 0xf0)>>4;
}

void illegal_inst(void)
{
int i;

    // If running we stop here
    step_mode = true;
    wprintw(wConsole,"Illegal instruction: size: %d, addr mode: %02X\n",app_size,adr_mode);
    for (i=0;i<11;i++)
    {
        wprintw(wConsole,"%02X ",app_memory[app_pc + i]);
    }
    wprintw(wConsole,"\n");
}

bool bool_xor(bool a, bool b)
{
    return (a && !b)||(!a && b);
}

uint8_t high(uint16_t value)
{
    return (value >> 8) & 0xff;
}

uint8_t low(uint16_t value)
{
    return value & 0xff;
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

    x = (a & 0xff) + (b & 0xff);
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

    x = (a & 0xff) + (b & 0xff) + get_cy();
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

    x = (a & 0xff) - (b & 0xff);
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

    x = (a & 0xff) - (b & 0xff) - get_cy();
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
//uint8_t temp;
    //temp = byte & 0x0f;
    //FIXME
    return 0;
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
    if (step_mode) wprintw(wConsole,"get_disp: Size: %d, app_pc: %04X, disp = %04X\n",app_size,app_pc & 0xffff,disp16 & 0xFFFF);
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
    app_memory[(sp-1)&0xffff] = (value >> 8) & 0xff;
    app_memory[(sp-2)&0xffff] = value & 0xff;
    setsp(sp-2);
    if (step_mode) wprintw(wConsole,"PUT RET ADDR: value: %04X, sp: %04X, H: %02X, L: %02X\n",value,sp & 0xffff,app_memory[sp-1],app_memory[sp-2]);
}

int16_t get_retaddr(void)
{
int16_t sp;

    sp = getsp();
    setsp(sp+2);

    if (step_mode) wprintw(wConsole,"RET: SP: %04X, H: %02X, L: %02X\n",sp & 0xffff,app_memory[(sp+1) & 0xffff],app_memory[sp & 0xffff]);
    return app_memory[(sp+1) & 0xffff]*256 + app_memory[sp & 0xffff];
}

void push_byte(uint8_t my_byte)
{
uint16_t sp;

    sp = (getsp() - 1) & 0xffff;

    if (step_mode) wprintw(wConsole,"push_byte:\tSP: %04X\n",sp);
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
uint8_t test_bit,test_mask,test;

    // Bit 3-1  flag
    // Bit 0    0:clear 1:set
    test_bit = (param & 0x0E) >> 1;
    test_mask = 1 << test_bit;
    test = (param & 0x01) << test_bit;

    if (step_mode) wprintw(wConsole,"BRANCH: bit: %d, mask: %02X, test: %d\n",test_bit, test_mask, test);

    if ((app_flags & test_mask) == test)
    {
        app_pc = app_pc + app_size + get_disp();
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
        app_pc = app_pc + app_size + get_disp();
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
    if (step_mode) wprintw(wConsole,"BSR %04X (%04X)\n",temp, app_pc);
}

void br_always(void)
{
int16_t temp;

    temp = get_disp();
    if (step_mode) wprintw(wConsole,"BR %04X (%04X) (%04X)\n",temp & 0xFFFF, app_pc & 0xFFFF, (app_pc+app_size+temp) & 0xFFFF);
    app_pc = app_pc + app_size + temp;
}

void bsr_cond(uint8_t param)
{
int16_t disp;
uint8_t test_bit,test_mask,test;

    // Bit 3-1  flag
    // Bit 0    0:clear 1:set
    test_bit = (param & 0x0E) >> 1;
    test_mask = 1 << test_bit;
    test = (param & 0x01) << test_bit;

    disp = get_disp();

    if (step_mode) wprintw(wConsole,"BSR: bit: %d, mask: %02X, test: %d, disp %04X\n",test_bit, test_mask, test, disp);

    if ((app_flags & test_mask) == test)
    {
        put_retaddr(app_pc + app_size); // next instruction after call
        app_pc += disp;
        if (step_mode) wprintw(wConsole,"BSR %04X (%04X)\n",disp, app_pc);
    }
    else
    {
        app_pc += app_size;
    }
}

void mcpdr(uint8_t param)
{
int i = get_addr(param);
uint16_t adr_src,adr_dest;

    getPair();
    adr_src = get_addr(src);
    adr_dest = get_addr(dest);

    do
    {
        app_memory[adr_dest--] = app_memory[adr_src--];
    } while (--i);

    // Update registers
    set_addr(param,i);
    set_addr(src,adr_src);
    set_addr(dest,adr_dest);
}

void mcpir(uint8_t param)
{
int i = get_addr(param);
uint16_t adr_src,adr_dest;

    getPair();
    adr_src = get_addr(src);
    adr_dest = get_addr(dest);

    wprintw(wConsole,"MCPIR: PC: %04X, pair: %02X, ctr: %02X\n",app_pc,reg_pair,param);

    do
    {
        app_memory[adr_dest++] = app_memory[adr_src++];
    } while (--i);

    // Update registers
    set_addr(param,i);
    set_addr(src,adr_src);
    set_addr(dest,adr_dest);
}

void mswap(uint8_t param)
{
int i = get_addr(param);
uint16_t adr_src,adr_dest;
uint8_t temp;

    getPair();
    adr_src = get_addr(src);
    adr_dest = get_addr(dest);

    wprintw(wConsole,"MSWAP: PC: %04X, pair: %02X, ctr: %02X\n",app_pc,reg_pair,param);

    do
    {
        temp = app_memory[adr_dest];
        app_memory[adr_dest++] = app_memory[adr_src];
        app_memory[adr_src++] = temp;
    } while (--i);

    // Update registers
    set_addr(param,i);
    set_addr(src,adr_src);
    set_addr(dest,adr_dest);
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
        wprintw(wConsole,"WAIT PORT,BIT,VALUE: [%02X.%d] = %d\n",port,bit,(param & 0x08) >> 3);
        result = scanw("%02X",&input);
        wprintw(wConsole,"Result: %02X, Input: %02X\n",result,input & 0xff);
        wprintw(wConsole,"Mask: %02X, Value: %02X\n",mask,value);
        if ((input & mask) == value) wprintw(wConsole,"Bit match\n");
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
    if (step_mode) wprintw(wConsole,"DJNZ %04X (%04X) (%04X)\n",temp & 0xFFFF, app_pc & 0xFFFF, (app_pc+app_size+temp) & 0xFFFF);

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
    if (adr_mode == 0)
    {
        op_dec(app_reg[param]);
    }
    else
    {
    }
}

void sfl(uint8_t param)
{
    if (step_mode) wprintw(wConsole,"SFL: param: %02X\n",param);
    if (adr_mode == 0)
    {
        setflags(app_reg[param]);
    }
    else
    {
    }
}

void push(uint8_t param)
{
    // MSB to LSB
    for (int i=app_size-1;i>=0;i--)
    {
        push_byte(app_reg[param][i]);
    }
}

void pop(uint8_t param)
{
    // LSB to MSB
    for (int i=0;i<app_size;i++)
    {
        app_reg[param][i] = pop_byte();
    }
}

void ind_call(uint8_t param)
{
    switch(adr_mode & 0xF0)
    {
        case 0xE0: // (r)
            put_retaddr(app_pc);
            app_pc = get_addr(param);
            break;
        case 0xF0: // (table)[r]
            put_retaddr(app_pc+2);
            //get table pointer (reuse app_pc)
            if (step_mode) wprintw(wConsole,"IND CALL (table): PC: %04X\n",app_pc);
            app_pc = getword(app_pc) + get_addr(param)*2;
            if (step_mode) wprintw(wConsole,"(table)[r]: %04X, %04X\n",app_pc,getword(app_pc));
            //get address from table
            app_pc = getword(app_pc);
            break;
        default:
            illegal_inst();
    }
}

void ind_bsr(uint8_t param)
{
    switch(adr_mode & 0xF0)
    {
        case 0xE0: // (r)
            put_retaddr(app_pc + 2); // next instruction after call
            app_pc = get_addr(param);
            break;
        case 0xF0: // (table)[r]
            switch(app_size)
            {
                case 1:
                    put_retaddr(app_pc + 1); // next instruction after call
                    wprintw(wConsole,"BSR (r): PC: %04X, MEM: %02X\n",app_pc,app_memory[app_pc]);
                    //get table pointer (reuse app_pc)
                    app_pc = app_pc +1 + app_memory[app_pc] + get_addr(param)*2;
                    wprintw(wConsole,"table: %04X\n",app_pc);
                    //get address from table
                    app_pc = getword(app_pc);
                    wprintw(wConsole,"value: %04X\n",app_pc);
                    break;
                case 2:
                    put_retaddr(app_pc + 2); // next instruction after call
                    //get table pointer (reuse app_pc)
                    app_pc = app_pc + getword(app_pc) + get_addr(param)*2;
                    //get address from table
                    app_pc = getword(app_pc);
                    break;
                default:
                    illegal_inst();
            }
//            app_pc = getword(app_pc) + get_addr(src)*2;
            break;
        default:
            illegal_inst();
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
                wprintw(wConsole,"Waiting for bit %d of reg %d to be %d\n",bit,reg,val>>7);
                getchar();
            }
            break;
        default:
            illegal_inst();
    }
}

void op_rand(void)
{
int i;

    for (i=0;i<app_size;i++)
    {
        app_reg[0][i] = rand() & 0xFF;
    }
}

void push_imm(void)
{
int i;

    for (i=0;i<app_size;i++)
    {
        push_byte(app_memory[(app_pc++) & 0xffff]);
    }
}

void popn(void)
{
    setsp(getsp() + app_memory[(app_pc++) & 0xffff]);
}

void mfill(void)
{
uint8_t value;
uint16_t begin,end,i;

    getPair();
    begin = get_addr(src);
    end = get_addr(dest);
    value = app_memory[(app_pc++) & 0xffff];

    for (i = begin; i <= end; i++)
    {
        app_memory[i] = value;
    }
}
