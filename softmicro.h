#ifndef _SOFTMICRO_H_
#define _SOFTMICRO_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define VECTOR_TABLE_BEGIN 0xFFE0

void illegal_inst(void);

bool bool_xor(bool a, bool b);

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


/* reg ops */
void setflags(uint8_t reg[]);
void op_load(void);
void op_ex(void);
void op_add(void);
void op_adc(void);
void op_sub(void);
void op_sbc(void);
void op_cp(void);
void op_and(void);
void op_or(void);
void op_xor(void);
void op_inc(uint8_t reg[]);
void op_in(void);
void op_out(void);
void op_asx(void);
void op_rox(void);
void op_lsx(void);

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

void wait_port(uint8_t param);
void app_vector(uint8_t param);
void djnz(uint8_t param);
void clr(uint8_t param);
void inc(uint8_t param);
void dec(uint8_t param);

uint8_t set_bit_val(bool val, uint8_t bit, uint8_t reg);
uint8_t get_bit_val(uint8_t bit, uint8_t reg);
void test_bit_reg(uint8_t bit, uint8_t reg);

void set_bit(void);
void toggle_bit(void);
void test_bit(void);

void OpStep(void);
void OpStep2(void);
void OpStep3(void);
void OpExtended(void);

extern uint8_t app_memory[65536];
extern uint8_t app_reg[16][16];
extern uint8_t app_flags;
extern int16_t app_pc;
extern uint8_t app_size,adr_mode;

#define FLAG_S_MASK 0x80
#define FLAG_T_MASK 0x40
#define FLAG_H_MASK 0x20
#define FLAG_I_MASK 0x10
#define FLAG_V_MASK 0x08
#define FLAG_N_MASK 0x04
#define FLAG_Z_MASK 0x02
#define FLAG_C_MASK 0x01

#define FLAG_S_BIT 7
#define FLAG_T_BIT 6
#define FLAG_H_BIT 5
#define FLAG_I_BIT 4
#define FLAG_V_BIT 3
#define FLAG_N_BIT 2
#define FLAG_Z_BIT 1
#define FLAG_C_BIT 0

#endif /* _SOFTMICRO_H_ */

