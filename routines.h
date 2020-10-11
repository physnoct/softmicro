#ifndef _ROUTINES_H_
#define _ROUTINES_H_

extern uint8_t reg_pair,src,dest;

void getPair(void);
void illegal_inst(void);

bool bool_xor(bool a, bool b);

uint8_t high(uint16_t value);
uint8_t low(uint16_t value);

uint8_t get_cy(void);
void set_cy(uint8_t flags);

int8_t byte_add(int8_t a, int8_t b);
int8_t byte_adc(int8_t a, int8_t b);
int8_t byte_sub(int8_t a, int8_t b);
int8_t byte_sbc(int8_t a, int8_t b);

uint8_t swap_byte(uint8_t byte);
uint8_t reverse_byte(uint8_t byte);
uint8_t daa_byte(uint8_t byte);
uint8_t toa_byte(uint8_t byte);
uint8_t toh_byte(uint8_t byte);
uint8_t bcd_byte(uint8_t byte);
uint8_t bin_byte(uint8_t byte);
uint8_t sxt_byte(void);

int16_t get_addr(uint8_t reg);
int16_t get_addr_disp8(uint8_t reg,uint8_t disp8);
void set_addr(uint8_t reg, int16_t addr);
int16_t get_disp(void);
int16_t getword(uint16_t addr);
int16_t getsp(void);
void setsp(int16_t value);
void put_retaddr(int16_t value);
int16_t get_retaddr(void);
void push_byte(uint8_t my_byte);
uint8_t pop_byte(void);

void branch(uint8_t param);
void branch2(uint8_t opcode);
void br_sbr(void);
void br_always(void);
void bsr_cond(uint8_t param);
void mcpdr(uint8_t param);
void mcpir(uint8_t param);
void mswap(uint8_t param);

void wait_port(uint8_t param);
void app_vector(uint8_t param);
void djnz(uint8_t param);
void clr(uint8_t param);
void inc(uint8_t param);
void dec(uint8_t param);
void sfl(uint8_t param);
void push(uint8_t param);
void pop(uint8_t param);
void ind_call(uint8_t param);
void ind_bsr(uint8_t param);

uint8_t set_bit_val(bool val, uint8_t bit, uint8_t reg);
uint8_t get_bit_val(uint8_t bit, uint8_t reg);
void test_bit_reg(uint8_t bit, uint8_t reg);

void set_bit(void);
void toggle_bit(void);
void test_bit(void);
void wait_bit(void);
void op_rand(void);
void push_imm(void);
void popn(void);
void mfill(void);

#endif // _ROUTINES_H_

