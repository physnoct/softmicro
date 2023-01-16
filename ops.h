#ifndef _OPS_H_
#define _OPS_H_

/* reg ops */
void setflags(uint8_t reg[]);
void op_rev(void);
void op_sxt(void);
void op_cpl(void);
void op_neg(void);
void op_st(void);
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
void op_msk(uint128_t *reg);
void op_swap(uint128_t *reg);
void op_inc(uint8_t reg[]);
void op_dec(uint8_t reg[]);
void op_in(void);
void op_out(void);
void op_asx(void);
void op_rox(void);
void op_lsx(void);
void op_umul(void);
void op_udiv(void);
void op_smul(void);
void op_sdiv(void);

#endif // _OPS_H_
