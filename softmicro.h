#ifndef _SOFTMICRO_H_
#define _SOFTMICRO_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <ncurses.h>

#define SOFT_MICRO_INST_SET_VERSION 3
#define VECTOR_TABLE_BEGIN 0xFFE0
#define NMI_VECT 0xFFDE

typedef union {
    uint8_t B[16];
    uint16_t W[8];
    uint32_t D[4];
    uint64_t Q[2];
#ifdef __SIZEOF_INT128__
    __uint128_t O;
#endif
} uint128_t;

#include "ops.h"
#include "routines.h"
#include "decode.h"

extern WINDOW *wMain, *wRegs, *wMem, *wStack, *wPC, *wConsole;

void OpStep(void);
void OpStep2(void);
void OpStep3(void);
void OpExtended(void);

extern uint8_t hvect;
extern uint8_t app_memory[65536];
extern uint128_t app_reg[16];
extern uint8_t app_flags;
extern uint16_t app_pc;
extern uint8_t app_size,adr_mode,size_byte;
extern bool step_mode;
extern bool run_until_ret;
extern bool extended;

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

