/*
 * decode.h
 *
 *  Created on: 17 déc. 2022
 *      Author: jpellet
 */

#ifndef DECODE_H_
#define DECODE_H_

extern char decode_buffer[80];
extern char dump_buffer[80];

void OpDecode(uint16_t address);

#endif /* DECODE_H_ */
